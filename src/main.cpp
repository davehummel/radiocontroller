#include <Arduino.h>
#include <SPI.h>
#include <i2c_t3.h>

#include <RadioLib.h> //Click here to get the library: http://librarymanager/All#RadioLib

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library

#include "FDOS_LOG.h"
#include "RemoteDisplay.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include "RadioTask.h"

#define SPI_CLK_PIN 27
#define SPI_MOSI_PIN 11
#define TFT_CS_PIN 10 // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define TFT_DC_PIN 2
//  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define TFT_RST_PIN 4
#define TFT_PWM_PIN 3
Adafruit_ST7789 tft(&SPI, TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);

#define RADIO_CS_PIN 9
#define RADIO_DIO0_PIN 26
#define RADIO_DIO1_PIN 28
#define RADIO_RST_PIN 29
#define RADIO_TX_EN_PIN 24
#define RADIO_RX_EN_PIN 25
SX1276 radio(new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN));

#define POWER_LED_PIN 13

#define BAT_V_SENSE_PIN 33
#define BAT_V_MIN_LVL 3.1

#define BAT_SENSE_ENABLE_PIN 35
#define POWER_ENABLE_SENSE_PIN 34
#define POWER_PRESS_SENSE_LVL 700

#define JOY_BTN_PIN 30
#define JOY_H_PIN 31
#define JOY_V_PIN 32

#define H_SLIDER_PIN 23
#define V_SLIDER_PIN 22

#define BTN1_LED_PIN 5
#define BTN2_LED_PIN 6
#define BTN3_LED_PIN 7
#define BTN4_LED_PIN 8

#define BTN1_PRESS_PIN 17
#define BTN2_PRESS_PIN 16
#define BTN3_PRESS_PIN 15
#define BTN4_PRESS_PIN 14

#define TOGGLE1_PIN 36
#define TOGGLE2_PIN 21

Logger FDOS_LOG(&Serial);

VMExecutor executor;

RemoteUI ui;

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
            digitalWrite(POWER_LED_PIN, HIGH);
            delay(msOn);
            digitalWrite(POWER_LED_PIN, LOW);
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
                digitalWrite(POWER_LED_PIN, LOW);
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
            ui.renderTxBattery(batteryPercent);
            if (bat_v < 2.0) { // Assume we are running on external power
                FDOS_LOG.print(" WARNING - Assuming on external power.");
            } else if (bat_v < BAT_V_MIN_LVL) {
                FDOS_LOG.print("WARNING - Bat V low, turning off!");

                for (int i = 0; i < 10; i++) {
                    digitalWrite(POWER_LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(POWER_LED_PIN, LOW);
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
    joyButton = 1 << 7,
    button1 = 1 << 8,
    button2 = 1 << 9,
    button3 = 1 << 10,
    button4 = 1 << 11
};

uint16_t getChangeMap(inputs_t &prev, inputs_t &next) {
    uint16_t map = 0;
    map |= (prev.joyH != next.joyH) * INPUT_CHANGE_MAP::joyH;
    map |= (prev.joyV != next.joyV) * INPUT_CHANGE_MAP::joyV;
    map |= (prev.slideV != next.slideV) * INPUT_CHANGE_MAP::slideV;
    map |= (prev.slideH != next.slideH) * INPUT_CHANGE_MAP::slideH;
    map |= (prev.toggle1 != next.toggle1) * INPUT_CHANGE_MAP::toggle1;
    map |= (prev.toggle2 != next.toggle2) * INPUT_CHANGE_MAP::toggle2;
    map |= (prev.joyButton != next.joyButton) * INPUT_CHANGE_MAP::joyButton;
    map |= (prev.button1 != next.button1) * INPUT_CHANGE_MAP::button1;
    map |= (prev.button2 != next.button2) * INPUT_CHANGE_MAP::button2;
    map |= (prev.button3 != next.button3) * INPUT_CHANGE_MAP::button3;
    map |= (prev.button4 != next.button4) * INPUT_CHANGE_MAP::button4;
}

class InputTask : public RunnableTask {
  private:
    uint8_t printCycle = 0;

  public:
    int16_t joyH_offset = -481;
    int16_t joyV_offset = -516;
    int16_t joyH_deadzone = 8;  //+-
    int16_t joyV_deadzone = 18; // +-
    int16_t joyH_range = 450;
    int16_t joyV_range = 450;

    int16_t slideH_offset = 0;
    int16_t slideV_offset = 0;
    int16_t slideH_range = 1023;
    int16_t slideV_range = 1023;

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
        delayMicroseconds(1);
        int32_t sliderHRaw1 = analogRead(H_SLIDER_PIN);
        pinMode(H_SLIDER_PIN, INPUT_PULLUP);
        delayMicroseconds(1);
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
        FDOS_LOG.printf("Btns J:%i 1:%i 2:%i 3:%i 4:%i Joy v:%i h:%i Sld v:%i h:%i Sw 1:%i 2:%i\n", inputs.joyButton, inputs.button1, inputs.button2,
                        inputs.button3, inputs.button4, inputs.joyV, inputs.joyH, inputs.slideV, inputs.slideH, inputs.toggle1, inputs.toggle2);
    }

} inputTask;

void beginReceive(void) { radio.startReceive(); }

RadioTask radioTask((PhysicalLayer *)&radio, 50000ULL, beginReceive);

void radioSendInterrupt(void) { radioTask.interruptTriggered(); }

void radioReceiveInterrupt(void) { radioTask.interruptTriggered(); }

void startRadioActions();

class ControlOutputAction : RadioAction, RunnableTask {

    ScheduledLink *cancel = NULL;

    bool flightModeActive = false;

    inputs_t knownInput;

    void run(TIME_INT_t time) {
        inputs_t newInput = inputTask.inputs;
        uint16_t stateChanges = getChangeMap(knownInput, newInput);
        knownInput = newInput;

        if (flightModeActive) {
            if (stateChanges && INPUT_CHANGE_MAP::button1) {
                digitalWrite(BTN1_LED_PIN, LOW);
                flightModeActive = false;
                requestSend();
                return;
            } else {
                digitalWrite(BTN1_LED_PIN, HIGH);
                if (stateChanges & (INPUT_CHANGE_MAP::joyH | INPUT_CHANGE_MAP::joyV | INPUT_CHANGE_MAP::slideH | INPUT_CHANGE_MAP::slideV)) {
                    requestSend();
                }
            }
        } else {
            if (stateChanges && INPUT_CHANGE_MAP::button1) {
                digitalWrite(BTN1_LED_PIN, HIGH);
                flightModeActive = true;
                return;
            } else {
                digitalWrite(BTN1_LED_PIN, time / 1000 % 10 <= 3);
            }
        }
    }

    uint8_t onSendReady(uint8_t *data) {
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
        return 5;
    }

    void onStart() {
        knownInput = inputTask.inputs;
        if (cancel != NULL)
            cancel->cancel();
        cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(MAX_TRANSMIT_PER_SECOND, FrequencyUnitEnum::second));
    }

    void onStop() {
        if (cancel != NULL)
            cancel->cancel();
        cancel = NULL;
        digitalWrite(BTN1_LED_PIN, LOW);
    }
}controlOutputAction;

class SustainConnectionAction : RadioAction, RunnableTask {

    uint8_t missedHBCount = 0;

    uint8_t rxBat = 255;

    ScheduledLink *cancel = NULL;

    void onReceive(uint8_t length, uint8_t *data) {
        if (data[0] == HEARTBEAT) {
            FDOS_LOG.println("Heartbeat received from receiver");
            rxBat = data[1];
            int8_t rxSNR = data[2];
            int16_t txSNR = radio.getSNR();

            radioTask->getUI()->renderRxBattery(rxBat);
            radioTask->getUI()->renderRadioState(2, rxSNR, txSNR);

            FDOS_LOG.printf("Radio Stats - rxBat %i, rxSNR %i, txSNR %i", rxBat, rxSNR, txSNR);
            missedHBCount--;
            return;
        }
        if (data[0] == RADIO_MSG_ID::RECEIVER_BEACON) {
            FDOS_LOG.println("Beacon arrived from Receiver, killing current connection");
            missedHBCount = 6;
        }
    }

    // returns length of data to send
    uint8_t onSendReady(uint8_t *data) {
        data[0] = RADIO_MSG_ID::HEARTBEAT;
        data[1] = powerTask.getBatteryPercent();
        data[2] = radio.getSNR();
        FDOS_LOG.println("Heartbeat sent to receiver");
        return 3;
    }

    void run(TIME_INT_t time) {
        if (missedHBCount > 5) {
            FDOS_LOG.println("DISCONNECTING RECEIVER - No heartbeat in 6 cycles");
            radioTask->removeAllActions();

            startRadioActions();

            return;
        } else {
            if (missedHBCount > 0)
                radioTask->getUI()->renderRadioState(2, 0, missedHBCount);
            if (requestSend()) {
                missedHBCount++;
            } else {
                FDOS_LOG.println("***WARNING*** Can't send HB signal");
            }
        }
    }

    void onStart() {
        FDOS_LOG.println("Connection established");
        missedHBCount = 0;
        if (cancel != NULL)
            cancel->cancel();
        radioTask->getUI()->renderRadioState(2, 0, 0);
        cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(TRANSMITTER_HB_SECONDS, FrequencyUnitEnum::second));
    }

    void onStop() {
        rxBat = 255;
        if (cancel != NULL)
            cancel->cancel();
        cancel = NULL;
    }

  public:
    uint8_t receiverId = 0;

    uint8_t getReceiverBattery() { return rxBat; }

} sustainConnectionAction;

class FindReceiverAction : RadioAction, RunnableTask {

    ScheduledLink *cancel = NULL;

    uint8_t receiverId = 0;
    uint8_t confirmingClient = 0;

    void onStart() {
        radioTask->getUI()->renderRadioState(0, 0, 0);
        radioTask->getUI()->renderRxBattery(255);
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
            radioTask->getUI()->renderRadioState(1, 0, 0);
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
            radioTask->getUI()->renderRadioState(0, x, 0);
        }
    }

} findReceiver;

void startRadioActions() { radioTask.addAction((RadioAction *)&findReceiver); }

void setup(void) {
    SPI.setSCK(SPI_CLK_PIN);

    // PowerEnable pin must be enabled ASAP to keep system on after power is
    // released
    pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
    digitalWrite(POWER_ENABLE_SENSE_PIN, HIGH); // Power Enable
    pinMode(BAT_SENSE_ENABLE_PIN, OUTPUT);
    pinMode(BAT_V_SENSE_PIN, INPUT);
    pinMode(POWER_LED_PIN, OUTPUT);
    digitalWrite(POWER_LED_PIN, true);

    pinMode(BTN1_LED_PIN, OUTPUT);
    pinMode(BTN2_LED_PIN, OUTPUT);
    pinMode(BTN3_LED_PIN, OUTPUT);
    pinMode(BTN4_LED_PIN, OUTPUT);

    Serial.begin(921600);

    tft.init(240, 240);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_WHITE);
    tft.setTextColor(ST77XX_BLACK);
    delay(1000);
    tft.setCursor(0, 0);
    tft.print("System Starting ...");

    FDOS_LOG.println("System Starting ...");

    ui.setDisplay(&tft);

    tft.println("[SX1276] Initializing ...");
    int state = radio.begin(910.0, 500); //-23dBm
    if (state == RADIOLIB_ERR_NONE) {
        tft.println("success!");
    } else {
        tft.print("SX1276 failed. Code:");
        tft.println(state);
        powerTask.bootFailed(200, 50);
    }
    // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    // NOTE: 20 dBm value allows high power operation, but transmission
    //       duty cycle MUST NOT exceed 1%
    if (radio.setOutputPower(17) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        tft.println("Invalid Power!");
        powerTask.bootFailed(50, 200);
    }
    radio.setRfSwitchPins(RADIO_RX_EN_PIN, RADIO_TX_EN_PIN);

    radio.setDio0Action(radioReceiveInterrupt);
    radio.setDio1Action(radioSendInterrupt);

    tft.println("Radio init success");

    analogReadResolution(10);
    analogWriteResolution(10);

    tft.println("Init success");

    executor.schedule((RunnableTask *)&powerTask, executor.getTimingPair(5, FrequencyUnitEnum::per_second));

    executor.schedule((RunnableTask *)&inputTask, executor.getTimingPair(40, FrequencyUnitEnum::per_second));

    executor.schedule((RunnableTask *)&radioTask, executor.getTimingPair(1, FrequencyUnitEnum::second));

    ui.renderBackground();

    radioTask.setUI((RadioUI *)&ui);
    startRadioActions();

    pinMode(TFT_PWM_PIN, OUTPUT);
    analogWrite(TFT_PWM_PIN, 100);
}

void loop(void) {
    uint32_t delayTime = executor.runSchedule();
    if (delayTime > 10000)
        delayTime = 10000;
    if (delayTime > MIN_MICRO_REST)
        delayMicroseconds(delayTime - MIN_MICRO_REST);
}
