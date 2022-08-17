#include <Arduino.h>
#include <SPI.h>
#include <i2c_t3.h>

#include "PowerControl.h"


#include <RadioLib.h> //Click here to get the library: http://librarymanager/All#RadioLib
#include <U8g2lib.h>


#include "FDOS_LOG.h"

// #include "RemoteDisplay.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include "FlightMessages.h"
#include "RadioTask.h"

#include "InputControls.h"


U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

SX1276 radio(new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN));
void beginReceive(void) { radio.startReceive(); }



Logger FDOS_LOG(&Serial);

VMExecutor executor;


// RemoteUI ui;

class PowerManagementTask : public RunnableTask {
  private:
    int16_t sampleCounter = -1; // First sample cycle needs to run one
                                // extra to fill in the sliding average
    boolean powerInitallyReleased = false;
    uint32_t batteryVSense = 0;
    uint8_t batteryPercent = 255;

  public:
    void bootFailed(uint16_t msOff, uint16_t msOn) {
        while (true) {
            digitalWrite(LED_PIN, HIGH);
            delay(msOn);
            digitalWrite(LED_PIN, LOW);
            delay(msOff);
            testPowerPress();
        }
    }

    uint8_t getBatteryPercent() { return batteryPercent; }

    void testPowerPress() {
        pinMode(POWER_ENABLE_SENSE_PIN, INPUT_PULLUP);
        delayMicroseconds(1);
        bool powerPressed = analogRead(POWER_ENABLE_SENSE_PIN) > POWER_PRESS_SENSE_LVL;
        pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
        pinMode(POWER_ENABLE_SENSE_PIN, HIGH); // Keep on
        if (!powerPressed && !powerInitallyReleased)
            powerInitallyReleased = true;
        if (powerPressed && powerInitallyReleased) {
            delay(200); // Verify its held down for 200 ms longer,
                        // filter out accidental bumps
            pinMode(POWER_ENABLE_SENSE_PIN, INPUT_PULLUP);
            delayMicroseconds(1);
            powerPressed = analogRead(POWER_ENABLE_SENSE_PIN) > POWER_PRESS_SENSE_LVL;
            if (powerPressed) {
                FDOS_LOG.println("Turning off");
                digitalWrite(LED_PIN, LOW);
                pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
                pinMode(POWER_ENABLE_SENSE_PIN, LOW); // Turn off
                delay(10000);                         // Need time to allow voltage to drop
            } else {
                FDOS_LOG.println("ignoring short press");
                pinMode(POWER_ENABLE_SENSE_PIN,
                        OUTPUT); // Keep on & ignore press
                pinMode(POWER_ENABLE_SENSE_PIN, HIGH);
            }
        }
    }

    void run(TIME_INT_t time) {
        digitalWrite(BAT_SENSE_ENABLE_PIN, HIGH);

        batteryVSense += analogRead(BAT_V_SENSE_PIN);

        digitalWrite(BAT_SENSE_ENABLE_PIN, LOW);

        testPowerPress();

        if (sampleCounter == 9) {
            batteryVSense /= 11; // double readings and 1 of 5 record is the
                                 // rolling average from previous reading
            FDOS_LOG.print("***POWER*** Bat V:");
            float bat_v = (batteryVSense + 421.9545998) / 305.3763441;
            FDOS_LOG.print(bat_v);
            sampleCounter = 0;
            batteryPercent = 100 * (1 - (4.2 - bat_v) / (4.2 - BAT_V_MIN_LVL));
            ui.renderCTBattery(batteryPercent);
            if (bat_v < 2.0) { // Assume we are running on external power
                FDOS_LOG.print(" WARNING - Assuming on external power.");
            } else if (bat_v < BAT_V_MIN_LVL) {
                FDOS_LOG.print("WARNING - Bat V low, turning off!");

                for (int i = 0; i < 10; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
                    delay(150);
                }
                pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
                pinMode(POWER_ENABLE_SENSE_PIN, LOW); // Turn off
                delay(10000);                         // Need time to allow voltage to drop
            }
            FDOS_LOG.println();
        } else {
            sampleCounter++;
        }
    }
} powerTask;

struct inputs_t {
    int8_t joyH = 0;
    int8_t joyV = 0;

    uint8_t slideV = 0;
    uint8_t slideH = 0;

    uint8_t toggle1 = 0;
    uint8_t toggle2 = 0;

    bool joyButton = false;
    bool button1 = false;
    bool button2 = false;
    bool button3 = false;
    bool button4 = false;

    TIME_INT_t lastJoyButtonPressTime = 0;
    TIME_INT_t lastButton1PressTime = 0;
    TIME_INT_t lastButton2PressTime = 0;
    TIME_INT_t lastButton3PressTime = 0;
    TIME_INT_t lastButton4PressTime = 0;

    TIME_INT_t lastJoyButtonReleaseTime = 0;
    TIME_INT_t lastButton1ReleaseTime = 0;
    TIME_INT_t lastButton2ReleaseTime = 0;
    TIME_INT_t lastButton3ReleaseTime = 0;
    TIME_INT_t lastButton4ReleaseTime = 0;
};

enum INPUT_CHANGE_MAP {
    joyH = 1 << 0,
    joyV = 1 << 1,
    slideV = 1 << 2,
    slideH = 1 << 4,
    toggle1 = 1 << 5,
    toggle2 = 1 << 6,
    joyButtonDown = 1 << 7,
    button1Down = 1 << 8,
    button2Down = 1 << 9,
    button3Down = 1 << 10,
    button4Down = 1 << 11,
    joyButtonUp = 1 << 12,
    button1Up = 1 << 13,
    button2Up = 1 << 14,
    button3Up = 1 << 15,
    button4Up = 1 << 16
};

uint32_t getChangeMap(inputs_t &prev, inputs_t &next) {
    uint16_t map = 0;
    map |= (next.joyH == 0 && prev.joyH != 0) || (abs(prev.joyH - next.joyH) >= 2) * INPUT_CHANGE_MAP::joyH;
    map |= (next.joyV == 0 && prev.joyV != 0) || (abs(prev.joyV - next.joyV) >= 2) * INPUT_CHANGE_MAP::joyV;
    map |= (next.slideV == 0 && prev.slideV != 0) || (abs(prev.slideV - next.slideV) >= 2) * INPUT_CHANGE_MAP::slideV;
    map |= (prev.slideH != next.slideH) * INPUT_CHANGE_MAP::slideH;
    map |= (prev.toggle1 != next.toggle1) * INPUT_CHANGE_MAP::toggle1;
    map |= (prev.toggle2 != next.toggle2) * INPUT_CHANGE_MAP::toggle2;
    map |= (prev.lastJoyButtonPressTime != next.lastJoyButtonPressTime) * INPUT_CHANGE_MAP::joyButtonDown;
    map |= (prev.lastButton1PressTime != next.lastButton1PressTime) * INPUT_CHANGE_MAP::button1Down;
    map |= (prev.lastButton2PressTime != next.lastButton2PressTime) * INPUT_CHANGE_MAP::button2Down;
    map |= (prev.lastButton3PressTime != next.lastButton3PressTime) * INPUT_CHANGE_MAP::button3Down;
    map |= (prev.lastButton4PressTime != next.lastButton4PressTime) * INPUT_CHANGE_MAP::button4Down;
    map |= (prev.lastJoyButtonReleaseTime != next.lastJoyButtonReleaseTime) * INPUT_CHANGE_MAP::joyButtonDown;
    map |= (prev.lastButton1ReleaseTime != next.lastButton1ReleaseTime) * INPUT_CHANGE_MAP::button1Up;
    map |= (prev.lastButton2ReleaseTime != next.lastButton2ReleaseTime) * INPUT_CHANGE_MAP::button2Up;
    map |= (prev.lastButton3ReleaseTime != next.lastButton3ReleaseTime) * INPUT_CHANGE_MAP::button3Up;
    map |= (prev.lastButton4ReleaseTime != next.lastButton4ReleaseTime) * INPUT_CHANGE_MAP::button4Up;
    return map;
}

class InputTask : public RunnableTask {
  private:
    uint8_t printCycle = 0;

  public:
    int16_t joyH_offset = -481;
    int16_t joyV_offset = -516;
    int16_t joyH_deadzone = 20; //+-
    int16_t joyV_deadzone = 20; // +-
    int16_t joyH_range = 450;
    int16_t joyV_range = 450;

    int16_t slideH_offset = 0;
    int16_t slideV_offset = 0;
    int16_t slideH_range = 1023;
    int16_t slideV_range = 1023;

    uint8_t slideVSamples[4] = {0};
    uint8_t slideVSampleID = 0;

    inputs_t inputs;

    InputTask() {
        pinMode(JOY_BTN_PIN, INPUT_PULLUP);
        pinMode(JOY_H_PIN, INPUT_PULLDOWN);
        pinMode(JOY_V_PIN, INPUT_PULLDOWN);

        pinMode(TOGGLE1_PIN, INPUT_PULLUP);

        pinMode(BTN1_PRESS_PIN, INPUT_PULLUP);
        pinMode(BTN2_PRESS_PIN, INPUT_PULLUP);
        pinMode(BTN3_PRESS_PIN, INPUT_PULLUP);
        pinMode(BTN4_PRESS_PIN, INPUT_PULLUP);

        pinMode(H_SLIDER_PIN, INPUT_PULLDOWN);
        pinMode(V_SLIDER_PIN, INPUT);
    }

    void updateButtons(TIME_INT_t &bPTime, TIME_INT_t &bRTime, bool &bState, bool newState, TIME_INT_t time) {
        if (newState != bState) {
            if (bState == false)
                bPTime = time;
            else
                bRTime = time;
            bState = newState;
        }
    }

    void run(TIME_INT_t time) {
        // buttons
        bool joyButton = !digitalRead(JOY_BTN_PIN);
        bool button1 = !digitalRead(BTN1_PRESS_PIN);
        bool button2 = !digitalRead(BTN2_PRESS_PIN);
        bool button3 = !digitalRead(BTN3_PRESS_PIN);
        bool button4 = !digitalRead(BTN4_PRESS_PIN);

        updateButtons(inputs.lastJoyButtonPressTime, inputs.lastJoyButtonReleaseTime, inputs.joyButton, joyButton, time);
        updateButtons(inputs.lastButton1PressTime, inputs.lastButton1ReleaseTime, inputs.button1, button1, time);
        updateButtons(inputs.lastButton2PressTime, inputs.lastButton2ReleaseTime, inputs.button2, button2, time);
        updateButtons(inputs.lastButton3PressTime, inputs.lastButton3ReleaseTime, inputs.button3, button3, time);
        updateButtons(inputs.lastButton4PressTime, inputs.lastButton4ReleaseTime, inputs.button4, button4, time);

        inputs.toggle1 = !digitalRead(TOGGLE1_PIN);

        {
            pinMode(TOGGLE2_PIN, INPUT_PULLDOWN);
            delayMicroseconds(1);
            bool read1 = digitalRead(TOGGLE2_PIN);
            pinMode(TOGGLE2_PIN, INPUT_PULLUP);
            delayMicroseconds(1);
            bool read2 = digitalRead(TOGGLE2_PIN);
            if (read1 != read2)
                inputs.toggle2 = 1;
            else if (read1 == 0)
                inputs.toggle2 = 0;
            else
                inputs.toggle2 = 2;
        }

        int32_t joyHRaw = analogRead(JOY_H_PIN) + joyH_offset;
        int32_t joyVRaw = analogRead(JOY_V_PIN) + joyV_offset;

        pinMode(H_SLIDER_PIN, INPUT_PULLDOWN);
        int32_t sliderHRaw1 = analogRead(H_SLIDER_PIN);
        pinMode(H_SLIDER_PIN, INPUT_PULLUP);
        int32_t sliderHRaw2 = analogRead(H_SLIDER_PIN);

        int32_t sliderHRaw;
        if (sliderHRaw1 < 400 && sliderHRaw2 > 600) {
            sliderHRaw = 0;
        } else {
            sliderHRaw = (sliderHRaw1 + sliderHRaw2) / 2 + slideH_offset;
        }

        int32_t sliderVRaw = analogRead(V_SLIDER_PIN) + slideV_offset;

        if (joyHRaw < joyH_deadzone && joyHRaw > -joyH_deadzone) {
            inputs.joyH = 0;
        } else {
            joyHRaw = joyHRaw * 127 / joyH_range;
            if (joyHRaw > 127)
                inputs.joyH = 127;
            else if (joyHRaw < -127)
                inputs.joyH = -127;
            else
                inputs.joyH = joyHRaw;
        }
        if (joyVRaw < joyV_deadzone && joyVRaw > -joyV_deadzone) {
            inputs.joyV = 0;
        } else {
            joyVRaw = joyVRaw * 127 / joyV_range;
            if (joyVRaw > 127)
                inputs.joyV = 127;
            else if (joyVRaw < -127)
                inputs.joyV = -127;
            else
                inputs.joyV = joyVRaw;
        }

        inputs.slideH = (sliderHRaw * 255) / slideH_range;
        inputs.slideV = (sliderVRaw * 255) / slideV_range;

        if (printCycle++ == 20) {
            printInputs();
            printCycle = 0;
        }
    }

    void printInputs() {

#ifdef INPUT_TRACKING
        FDOS_LOG.printf("Btns J:%i 1:%i 2:%i 3:%i 4:%i Joy v:%i h:%i Sld v:%i h:%i Sw 1:%i 2:%i\n", inputs.joyButton, inputs.button1, inputs.button2,
                        inputs.button3, inputs.button4, inputs.joyV, inputs.joyH, inputs.slideV, inputs.slideH, inputs.toggle1, inputs.toggle2);
#endif
    }

} inputTask;

class DisplayBrightnessTask : public RunnableTask {

    uint8_t prevState = 4;

    void run(TIME_INT_t time) {
        uint8_t newState = inputTask.inputs.toggle2;
        if (newState != prevState) {
            switch (newState) {
            case 0:
                analogWrite(TFT_PWM_PIN, 0);
                // tft.enableSleep(true);

                break;
            case 1:
                analogWrite(TFT_PWM_PIN, 200);
                // tft.enableSleep(false);
                break;
            case 2:
                analogWrite(TFT_PWM_PIN, 1024);
                // tft.enableSleep(false);
                break;
            }
            prevState = newState;
        }
    }
} displayBrightnessTask;

// Radio

void startReceive(void) { radio.startReceive(); }

RadioTask radioTask(&radio);

void radioInterrupt(void) { radioTask.interruptTriggered(); }

void startRadioActions();

time_t delaySend = false;
bool flightModeActive = false;

class ControlOutputAction : RadioAction, RunnableTask {

    enum FLIGHT_COMMAND { CHANGE_FLIGHT_MODE = 0, CHANGE_CONFIG, SEND_ORIENTATION_RESET, SEND_CONTROLS };

    ScheduledLink *cancel = NULL;

    bool communicatedFlightModeActive = false;

    FLIGHT_COMMAND command;

    inputs_t knownInput;

    uint32_t lastSendTime = 0;
    uint8_t lastSent[4];

    ct_config_t controlConfig;

    bool blink = false;

    void run(TIME_INT_t time) {
        if (delaySend > time) {
            FDOS_LOG.println("Ignoring input to give space for HB");
            return;
        }

        if (lastSendTime > 0) {
            if (millis() - lastSendTime > 250) {
                ui.renderInput(lastSent[0], lastSent[1], lastSent[2], lastSent[3]);
                lastSendTime = 0;
            }
        }
        inputs_t newInput = inputTask.inputs;
        uint16_t stateChanges = getChangeMap(knownInput, newInput);
        knownInput = newInput;

        if (stateChanges & INPUT_CHANGE_MAP::button1Down) {
            FDOS_LOG.println("Changing Flight Mode");
            flightModeActive = !flightModeActive;
            command = CHANGE_FLIGHT_MODE;
            analogWrite(BTN1_LED_PIN, 200);
            requestSend();
            return;
        }

        if (stateChanges & INPUT_CHANGE_MAP::button2Down || stateChanges & INPUT_CHANGE_MAP::button3Down || stateChanges & INPUT_CHANGE_MAP::button4Down) {
            command = CHANGE_CONFIG;
            if (stateChanges & INPUT_CHANGE_MAP::button2Down) {
                if (controlConfig.yawMode == 0)
                    controlConfig.yawMode = ct_config_t::CONTROL_MODE::DIRECT;
                else
                    controlConfig.yawMode--;
            }
            if (stateChanges & INPUT_CHANGE_MAP::button3Down) {
                if (controlConfig.pitchMode == 0)
                    controlConfig.pitchMode = ct_config_t::CONTROL_MODE::DIRECT;
                else
                    controlConfig.pitchMode--;
            }
            if (stateChanges & INPUT_CHANGE_MAP::button4Down) {
                if (controlConfig.rollMode == 0)
                    controlConfig.rollMode = ct_config_t::CONTROL_MODE::DIRECT;
                else
                    controlConfig.rollMode--;
            }

            analogWrite(BTN2_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.yawMode);
            analogWrite(BTN3_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.pitchMode);
            analogWrite(BTN4_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.rollMode);

            requestSend();
            return;
        }

        if (flightModeActive) {
            if (stateChanges & INPUT_CHANGE_MAP::joyButtonDown) {
                command = SEND_ORIENTATION_RESET;
                requestSend();
                return;
            }
            if (stateChanges & (INPUT_CHANGE_MAP::joyH | INPUT_CHANGE_MAP::joyV | INPUT_CHANGE_MAP::slideH | INPUT_CHANGE_MAP::slideV)) {
                command = SEND_CONTROLS;
                requestSend();
                return;
            }
        } else {
            analogWrite(BTN1_LED_PIN, 800 * (((time / 100000) % 10) < 2));
        }
    }

    uint8_t onSendReady(uint8_t *data) {
        if (delaySend > microsSinceEpoch()) {
            FDOS_LOG.println("@@@@ UTOH!");
        }

        switch (command) {
        case CHANGE_FLIGHT_MODE:
            data[0] = RADIO_MSG_ID::FLIGHT_MODE_UPDATE;
            data[1] = flightModeActive;
            FDOS_LOG.print("Sending Flight mode ");
            FDOS_LOG.println(data[1] ? "On" : "Off");
            return 2;

        case SEND_ORIENTATION_RESET:
            FDOS_LOG.println("@@#@#Sending orientation reset");
            data[0] = RADIO_MSG_ID::RESET_ORIENTATION;
            return 1;

        case CHANGE_CONFIG:
            FDOS_LOG.printf("@@@ SENT Control changes y:%i p:%i r:%i\n",controlConfig.yawMode,controlConfig.pitchMode,controlConfig.rollMode);
            data[0] = RADIO_MSG_ID::CHANGE_CONFIG;
            msgToBytes(&controlConfig, data + 1, controlConfig.size);
            return controlConfig.size+1;

        case SEND_CONTROLS:
            // FDOS_LOG.printf("Transmitting Controls JH:%i JV:%i SH:%i SV:%i\n", knownInput.joyH, knownInput.joyV, knownInput.slideH, knownInput.slideV);
            FDOS_LOG.print('.');
            data[0] = RADIO_MSG_ID::TRANSMIT_CONTROLS;
            if (flightModeActive) {
                data[1] = knownInput.joyH;
                data[2] = knownInput.joyV;
                data[3] = knownInput.slideH;
                data[4] = knownInput.slideV;
            } else {
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
                data[4] = 0;
            }
            if (lastSendTime == 0)
                lastSendTime = millis();
            lastSent[0] = data[1];
            lastSent[1] = data[2];
            lastSent[2] = data[3];
            lastSent[3] = data[4];
            return 5;
        }
    }

    void onStart() {
        flightModeActive = false;
        knownInput = inputTask.inputs;
        if (cancel != NULL)
            cancel->cancel();
        cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(MAX_TRANSMIT_PER_SECOND, FrequencyUnitEnum::per_second));
    }

    void onStop() {
        flightModeActive = false;
        if (cancel != NULL)
            cancel->cancel();
        cancel = NULL;
        digitalWrite(BTN1_LED_PIN, LOW);
        digitalWrite(BTN2_LED_PIN, LOW);
        digitalWrite(BTN3_LED_PIN, LOW);
        digitalWrite(BTN4_LED_PIN, LOW);
    }
} controlOutputAction;

class SustainConnectionAction : RadioAction, RunnableTask {

    uint8_t missedHBCount = 0;

    fc_heartbeat_t lastFCHB;
    ct_heartbeat_t lastCTHB;

    ScheduledLink *cancel = NULL;

    void onReceive(uint8_t length, uint8_t *data) {
        if (data[0] == FC_HEARTBEAT) {
            FDOS_LOG.println("Heartbeat received from flight computer");
            delaySend = 0;
            // radioTask->setActiveListen(false);
            msgFromBytes(&lastFCHB, data + 1, fc_heartbeat_t::size);
            lastFCHB.print((Print *)&FDOS_LOG);

            int8_t fcSNR = lastFCHB.snr;
            int8_t ctSNR = radio.getSNR() * 10;

            ui.renderFCBattery(lastFCHB.batV);
            ui.renderRadioState(2, fcSNR, ctSNR);
            ui.renderMotors(lastFCHB.speeds[0], lastFCHB.speeds[1], lastFCHB.speeds[2], lastFCHB.speeds[3]);
            ui.renderOrientation(convertHeading(lastFCHB.targetHeadings[0]), convertHeading(lastFCHB.targetHeadings[1]),
                                 convertHeading(lastFCHB.targetHeadings[2]), convertHeading(lastFCHB.headings[0]), convertHeading(lastFCHB.headings[1]),
                                 convertHeading(lastFCHB.headings[2]));

            FDOS_LOG.printf("Radio Stats - rxBat %i, rxSNR %i, txSNR %i\n", lastFCHB.batV, fcSNR, ctSNR);
            missedHBCount = 0;
            return;
        }
        if (data[0] == RADIO_MSG_ID::RECEIVER_BEACON) {
            FDOS_LOG.println("Beacon arrived from Receiver, killing current connection");
            missedHBCount = 6;
        }
    }

    // returns length of data to send
    uint8_t onSendReady(uint8_t *data) {
        FDOS_LOG.println("@@@ SENT HEARTBEAT");
        lastCTHB.flightModeEnabled = flightModeActive;
        lastCTHB.batV = powerTask.getBatteryPercent();
        lastCTHB.snr = radio.getSNR() * 10;
        data[0] = RADIO_MSG_ID::CT_HEARTBEAT;
        msgToBytes(&lastCTHB, data + 1, ct_heartbeat_t::size);
        return 4;
    }

    void run(TIME_INT_t time) {
        if (missedHBCount > HB_MISS_DC_LIMIT) {

            FDOS_LOG.println("DISCONNECTING RECEIVER - No heartbeat");
            radioTask->removeAllActions();
            startRadioActions();

            return;
        } else {
            if (missedHBCount > 0)
                ui.renderRadioState(2, 0, missedHBCount);
            if (requestSend()) {
                delaySend = time + MICROS_PER_MILLI * TRANSMITTER_HB_ECHO_DELAY_MILLIS;
                // radioTask->setActiveListen(true);
                missedHBCount++;
                FDOS_LOG.println("SENT HEARTBEAT");
            } else {
                FDOS_LOG.println("***WARNING*** Can't send HB signal");
            }
        }
    }

    void onStart() {
        FDOS_LOG.println("Connection established");
        // radioTask->setActiveListen(false);
        missedHBCount = 0;
        if (cancel != NULL)
            cancel->cancel();
        ui.renderRadioState(2, 0, 0);
        cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(TRANSMITTER_HB_SECONDS, FrequencyUnitEnum::second));
    }

    void onStop() {
        lastFCHB.batV = 255;
        // radioTask->setActiveListen(true);
        if (cancel != NULL)
            cancel->cancel();
        cancel = NULL;
    }

  public:
    uint8_t receiverId = 0;

    uint8_t getReceiverBattery() { return lastFCHB.batV; }

} sustainConnectionAction;

class FindReceiverAction : RadioAction, RunnableTask {

    ScheduledLink *cancel = NULL;

    uint8_t receiverId = 0;
    uint8_t confirmingClient = 0;

    void onStart() {
        ui.renderRadioState(0, 0, 0);
        ui.renderFCBattery(255);
        if (cancel != NULL)
            cancel->cancel();

        cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(1, FrequencyUnitEnum::per_second));
    }

    void onStop() {
        if (cancel != NULL)
            cancel->cancel();
        cancel = NULL;
    }

    void onReceive(uint8_t length, uint8_t *data) {
        if (confirmingClient > 0 && data[0] == RADIO_MSG_ID::RECEIVER_RECOGNIZE_CONFIRM && data[1] == TRANSMITTER_ID) {
            FDOS_LOG.println("Confirmation arrived from Receiver");
            radioTask->removeAction(this);
            sustainConnectionAction.receiverId = receiverId;
            radioTask->addAction((RadioAction *)&sustainConnectionAction);
            radioTask->addAction((RadioAction *)&controlOutputAction);
            return;
        }

        if (data[0] == RADIO_MSG_ID::RECEIVER_BEACON) {
            FDOS_LOG.println("Beacon arrived from Receiver, sending response");
            receiverId = data[1];
            ui.renderRadioState(1, 0, 0);
            confirmingClient = 1;
            requestSend();
        }
    }

    // returns length of data to send
    uint8_t onSendReady(uint8_t *data) {
        data[0] = RADIO_MSG_ID::TRANSMITTER_RECOGNIZE;
        data[1] = TRANSMITTER_ID;
        data[2] = receiverId;
        return 3;
    }

    void run(TIME_INT_t time) {
        if (confirmingClient > 0) {
            confirmingClient++;
            if (confirmingClient == 4) {
                confirmingClient = 0;
            }
        } else {
            uint8_t x = (time / 1000000) % 4;
            ui.renderRadioState(0, x, 0);
        }
    }

} findReceiver;

void startRadioActions() { radioTask.addAction((RadioAction *)&findReceiver); }

void setup(void) {
    // PowerEnable pin must be enabled ASAP to keep system on after power is
    // released
    pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
    digitalWrite(POWER_ENABLE_SENSE_PIN, HIGH); // Power Enable
    pinMode(BAT_SENSE_ENABLE_PIN, OUTPUT);
    pinMode(BAT_V_SENSE_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, true);

    SPI.begin();
    Serial.begin(921600);

    // tft.init(240, 240);

    // tft.setRotation(1);
    // tft.fillScreen(ST77XX_WHITE);
    // tft.setTextColor(ST77XX_BLACK);
    // tft.setCursor(0, 0);
    // tft.print("System Starting ...");

    FDOS_LOG.println("System Starting ...");

    // ui.setDisplay(&tft);

    // tft.println("[SX1276] Initializing ...");
    FDOS_LOG.println("[SX1276] Initializing ...");
    int state = radio.begin(RADIO_CARRIER_FREQ, RADIO_LINK_BANDWIDTH, RADIO_SPREADING_FACTOR); //-23dBm

    if (state == RADIOLIB_ERR_NONE) {
        FDOS_LOG.println("success!");
        // tft.println("success!");
    } else {
        // tft.print("SX1276 failed. Code:");
        // tft.println(state);
        FDOS_LOG.print("SX1276 failed. Code:");
        FDOS_LOG.println(state);
        powerTask.bootFailed(200, 50);
    }
    // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    // NOTE: 20 dBm value allows high power operation, but transmission
    //       duty cycle MUST NOT exceed 1%
    if (radio.setOutputPower(RADIO_POWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        // tft.println("Invalid Power!");
        FDOS_LOG.println("Invalid Power!");
        powerTask.bootFailed(50, 200);
    }
    radio.setRfSwitchPins(RADIO_RX_EN_PIN, RADIO_TX_EN_PIN);

    radio.setDio0Action(radioInterrupt);
    // tft.println("Init success");
    FDOS_LOG.println("Init success");

    delay(1000);

    executor.schedule((RunnableTask *)&displayBrightnessTask, 500000, 1000000);

    executor.schedule((RunnableTask *)&powerTask, executor.getTimingPair(5, FrequencyUnitEnum::per_second));

    executor.schedule((RunnableTask *)&inputTask, executor.getTimingPair(MAX_TRANSMIT_PER_SECOND, FrequencyUnitEnum::per_second));

    executor.schedule((RunnableTask *)&radioTask, executor.getTimingPair(RADIO_INTERVAL_MILLIS, FrequencyUnitEnum::milli));

    ui.renderBackground();

    startRadioActions();
}

void loop(void) {
    uint32_t delayTime = executor.runSchedule();
    // You may want to allow the loop to finish and avoid long delays
    //    if you are using background arduino features
    // if (delayTime > 100000)
    //     delayTime = 100000;
    if (delayTime > MIN_MICRO_REST) {
        delayMicroseconds(delayTime - MIN_MICRO_REST);
    }
}
