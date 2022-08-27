#include <Arduino.h>
#include <SPI.h>

#include <RadioLib.h> //Click here to get the library: http://librarymanager/All#RadioLib

#include "FDOS_LOG.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include "PowerControl.h"
#include "SettingsStore.h"

#include <U8g2lib.h>

#include <InputControls.h>
#include <RemoteControlInputs.h>

#include <RemoteDisplay.h>
#include <RootUI.h>
#include <Screens.h>

#include <RadioTask.h>



Logger FDOS_LOG(&Serial);

void saveRuntime() { SETTINGS.saveRuntimeSeconds(); }

U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

void clearDisplay() {
    UI.getDisplay()->clear();
    UI.requestDraw(true);
}
SX1276 RADIO=new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN, SPI, SPISettings(SPI_RADIO_FREQ, MSBFIRST, SPI_MODE0));


// void beginReceive(void) { RADIO.startReceive(); }

// void startReceive(void) { RADIO.startReceive(); }

 RadioTask RADIOTASK(&RADIO);

void radioInterrupt(void) { RADIOTASK.interruptTriggered(); }


// time_t delaySend = false;
// bool flightModeActive = false;

// class ControlOutputAction : RadioAction, RunnableTask {

//     enum FLIGHT_COMMAND { CHANGE_FLIGHT_MODE = 0, CHANGE_CONFIG, SEND_ORIENTATION_RESET, SEND_CONTROLS };

//     ScheduledLink *cancel = NULL;

//     bool communicatedFlightModeActive = false;

//     FLIGHT_COMMAND command;

//     inputs_t knownInput;

//     uint32_t lastSendTime = 0;
//     uint8_t lastSent[4];

//     ct_config_t controlConfig;

//     bool blink = false;

//     void run(TIME_INT_t time) {
//         if (delaySend > time) {
//             FDOS_LOG.println("Ignoring input to give space for HB");
//             return;
//         }

//         if (lastSendTime > 0) {
//             if (millis() - lastSendTime > 250) {
//                 ui.renderInput(lastSent[0], lastSent[1], lastSent[2], lastSent[3]);
//                 lastSendTime = 0;
//             }
//         }
//         inputs_t newInput = inputTask.inputs;
//         uint16_t stateChanges = getChangeMap(knownInput, newInput);
//         knownInput = newInput;

//         if (stateChanges & INPUT_CHANGE_MAP::button1Down) {
//             FDOS_LOG.println("Changing Flight Mode");
//             flightModeActive = !flightModeActive;
//             command = CHANGE_FLIGHT_MODE;
//             analogWrite(BTN1_LED_PIN, 200);
//             requestSend();
//             return;
//         }

//         if (stateChanges & INPUT_CHANGE_MAP::button2Down || stateChanges & INPUT_CHANGE_MAP::button3Down || stateChanges & INPUT_CHANGE_MAP::button4Down) {
//             command = CHANGE_CONFIG;
//             if (stateChanges & INPUT_CHANGE_MAP::button2Down) {
//                 if (controlConfig.yawMode == 0)
//                     controlConfig.yawMode = ct_config_t::CONTROL_MODE::DIRECT;
//                 else
//                     controlConfig.yawMode--;
//             }
//             if (stateChanges & INPUT_CHANGE_MAP::button3Down) {
//                 if (controlConfig.pitchMode == 0)
//                     controlConfig.pitchMode = ct_config_t::CONTROL_MODE::DIRECT;
//                 else
//                     controlConfig.pitchMode--;
//             }
//             if (stateChanges & INPUT_CHANGE_MAP::button4Down) {
//                 if (controlConfig.rollMode == 0)
//                     controlConfig.rollMode = ct_config_t::CONTROL_MODE::DIRECT;
//                 else
//                     controlConfig.rollMode--;
//             }

//             analogWrite(BTN2_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.yawMode);
//             analogWrite(BTN3_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.pitchMode);
//             analogWrite(BTN4_LED_PIN, 1024 / ct_config_t::CONTROL_MODE::DIRECT * controlConfig.rollMode);

//             requestSend();
//             return;
//         }

//         if (flightModeActive) {
//             if (stateChanges & INPUT_CHANGE_MAP::joyButtonDown) {
//                 command = SEND_ORIENTATION_RESET;
//                 requestSend();
//                 return;
//             }
//             if (stateChanges & (INPUT_CHANGE_MAP::joyH | INPUT_CHANGE_MAP::joyV | INPUT_CHANGE_MAP::slideH | INPUT_CHANGE_MAP::slideV)) {
//                 command = SEND_CONTROLS;
//                 requestSend();
//                 return;
//             }
//         } else {
//             analogWrite(BTN1_LED_PIN, 800 * (((time / 100000) % 10) < 2));
//         }
//     }

//     uint8_t onSendReady(uint8_t *data) {
//         if (delaySend > microsSinceEpoch()) {
//             FDOS_LOG.println("@@@@ UTOH!");
//         }

//         switch (command) {
//         case CHANGE_FLIGHT_MODE:
//             data[0] = RADIO_MSG_ID::FLIGHT_MODE_UPDATE;
//             data[1] = flightModeActive;
//             FDOS_LOG.print("Sending Flight mode ");
//             FDOS_LOG.println(data[1] ? "On" : "Off");
//             return 2;

//         case SEND_ORIENTATION_RESET:
//             FDOS_LOG.println("@@#@#Sending orientation reset");
//             data[0] = RADIO_MSG_ID::RESET_ORIENTATION;
//             return 1;

//         case CHANGE_CONFIG:
//             FDOS_LOG.printf("@@@ SENT Control changes y:%i p:%i r:%i\n",controlConfig.yawMode,controlConfig.pitchMode,controlConfig.rollMode);
//             data[0] = RADIO_MSG_ID::CHANGE_CONFIG;
//             msgToBytes(&controlConfig, data + 1, controlConfig.size);
//             return controlConfig.size+1;

//         case SEND_CONTROLS:
//             // FDOS_LOG.printf("Transmitting Controls JH:%i JV:%i SH:%i SV:%i\n", knownInput.joyH, knownInput.joyV, knownInput.slideH, knownInput.slideV);
//             FDOS_LOG.print('.');
//             data[0] = RADIO_MSG_ID::TRANSMIT_CONTROLS;
//             if (flightModeActive) {
//                 data[1] = knownInput.joyH;
//                 data[2] = knownInput.joyV;
//                 data[3] = knownInput.slideH;
//                 data[4] = knownInput.slideV;
//             } else {
//                 data[1] = 0;
//                 data[2] = 0;
//                 data[3] = 0;
//                 data[4] = 0;
//             }
//             if (lastSendTime == 0)
//                 lastSendTime = millis();
//             lastSent[0] = data[1];
//             lastSent[1] = data[2];
//             lastSent[2] = data[3];
//             lastSent[3] = data[4];
//             return 5;
//         }
//     }

//     void onStart() {
//         flightModeActive = false;
//         knownInput = inputTask.inputs;
//         if (cancel != NULL)
//             cancel->cancel();
//         cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(MAX_TRANSMIT_PER_SECOND, FrequencyUnitEnum::per_second));
//     }

//     void onStop() {
//         flightModeActive = false;
//         if (cancel != NULL)
//             cancel->cancel();
//         cancel = NULL;
//         digitalWrite(BTN1_LED_PIN, LOW);
//         digitalWrite(BTN2_LED_PIN, LOW);
//         digitalWrite(BTN3_LED_PIN, LOW);
//         digitalWrite(BTN4_LED_PIN, LOW);
//     }
// } controlOutputAction;

// class SustainConnectionAction : RadioAction, RunnableTask {

//     uint8_t missedHBCount = 0;

//     fc_heartbeat_t lastFCHB;
//     ct_heartbeat_t lastCTHB;

//     ScheduledLink *cancel = NULL;

//     void onReceive(uint8_t length, uint8_t *data) {
//         if (data[0] == FC_HEARTBEAT) {
//             FDOS_LOG.println("Heartbeat received from flight computer");
//             delaySend = 0;
//             // radioTask->setActiveListen(false);
//             msgFromBytes(&lastFCHB, data + 1, fc_heartbeat_t::size);
//             lastFCHB.print((Print *)&FDOS_LOG);

//             int8_t fcSNR = lastFCHB.snr;
//             int8_t ctSNR = radio.getSNR() * 10;

//             ui.renderFCBattery(lastFCHB.batV);
//             ui.renderRadioState(2, fcSNR, ctSNR);
//             ui.renderMotors(lastFCHB.speeds[0], lastFCHB.speeds[1], lastFCHB.speeds[2], lastFCHB.speeds[3]);
//             ui.renderOrientation(convertHeading(lastFCHB.targetHeadings[0]), convertHeading(lastFCHB.targetHeadings[1]),
//                                  convertHeading(lastFCHB.targetHeadings[2]), convertHeading(lastFCHB.headings[0]), convertHeading(lastFCHB.headings[1]),
//                                  convertHeading(lastFCHB.headings[2]));

//             FDOS_LOG.printf("Radio Stats - rxBat %i, rxSNR %i, txSNR %i\n", lastFCHB.batV, fcSNR, ctSNR);
//             missedHBCount = 0;
//             return;
//         }
//         if (data[0] == RADIO_MSG_ID::RECEIVER_BEACON) {
//             FDOS_LOG.println("Beacon arrived from Receiver, killing current connection");
//             missedHBCount = 6;
//         }
//     }

//     // returns length of data to send
//     uint8_t onSendReady(uint8_t *data) {
//         FDOS_LOG.println("@@@ SENT HEARTBEAT");
//         lastCTHB.flightModeEnabled = flightModeActive;
//         lastCTHB.batV = powerTask.getBatteryPercent();
//         lastCTHB.snr = radio.getSNR() * 10;
//         data[0] = RADIO_MSG_ID::CT_HEARTBEAT;
//         msgToBytes(&lastCTHB, data + 1, ct_heartbeat_t::size);
//         return 4;
//     }

//     void run(TIME_INT_t time) {
//         if (missedHBCount > HB_MISS_DC_LIMIT) {

//             FDOS_LOG.println("DISCONNECTING RECEIVER - No heartbeat");
//             radioTask->removeAllActions();
//             startRadioActions();

//             return;
//         } else {
//             if (missedHBCount > 0)
//                 ui.renderRadioState(2, 0, missedHBCount);
//             if (requestSend()) {
//                 delaySend = time + MICROS_PER_MILLI * TRANSMITTER_HB_ECHO_DELAY_MILLIS;
//                 // radioTask->setActiveListen(true);
//                 missedHBCount++;
//                 FDOS_LOG.println("SENT HEARTBEAT");
//             } else {
//                 FDOS_LOG.println("***WARNING*** Can't send HB signal");
//             }
//         }
//     }

//     void onStart() {
//         FDOS_LOG.println("Connection established");
//         // radioTask->setActiveListen(false);
//         missedHBCount = 0;
//         if (cancel != NULL)
//             cancel->cancel();
//         ui.renderRadioState(2, 0, 0);
//         cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(TRANSMITTER_HB_SECONDS, FrequencyUnitEnum::second));
//     }

//     void onStop() {
//         lastFCHB.batV = 255;
//         // radioTask->setActiveListen(true);
//         if (cancel != NULL)
//             cancel->cancel();
//         cancel = NULL;
//     }

//   public:
//     uint8_t receiverId = 0;

//     uint8_t getReceiverBattery() { return lastFCHB.batV; }

// } sustainConnectionAction;

// class FindReceiverAction : RadioAction, RunnableTask {

//     ScheduledLink *cancel = NULL;

//     uint8_t receiverId = 0;
//     uint8_t confirmingClient = 0;

//     void onStart() {
//         ui.renderRadioState(0, 0, 0);
//         ui.renderFCBattery(255);
//         if (cancel != NULL)
//             cancel->cancel();

//         cancel = executor.schedule((RunnableTask *)this, executor.getTimingPair(1, FrequencyUnitEnum::per_second));
//     }

//     void onStop() {
//         if (cancel != NULL)
//             cancel->cancel();
//         cancel = NULL;
//     }

//     void onReceive(uint8_t length, uint8_t *data) {
//         if (confirmingClient > 0 && data[0] == RADIO_MSG_ID::RECEIVER_RECOGNIZE_CONFIRM && data[1] == TRANSMITTER_ID) {
//             FDOS_LOG.println("Confirmation arrived from Receiver");
//             radioTask->removeAction(this);
//             sustainConnectionAction.receiverId = receiverId;
//             radioTask->addAction((RadioAction *)&sustainConnectionAction);
//             radioTask->addAction((RadioAction *)&controlOutputAction);
//             return;
//         }

//         if (data[0] == RADIO_MSG_ID::RECEIVER_BEACON) {
//             FDOS_LOG.println("Beacon arrived from Receiver, sending response");
//             receiverId = data[1];
//             ui.renderRadioState(1, 0, 0);
//             confirmingClient = 1;
//             requestSend();
//         }
//     }

//     // returns length of data to send
//     uint8_t onSendReady(uint8_t *data) {
//         data[0] = RADIO_MSG_ID::TRANSMITTER_RECOGNIZE;
//         data[1] = TRANSMITTER_ID;
//         data[2] = receiverId;
//         return 3;
//     }

//     void run(TIME_INT_t time) {
//         if (confirmingClient > 0) {
//             confirmingClient++;
//             if (confirmingClient == 4) {
//                 confirmingClient = 0;
//             }
//         } else {
//             uint8_t x = (time / 1000000) % 4;
//             ui.renderRadioState(0, x, 0);
//         }
//     }

// } findReceiver;

// void startRadioActions() { radioTask.addAction((RadioAction *)&findReceiver); }

void setup(void) {
    Serial.begin(921600);
    SPI.begin();
    u8g2.setBusClock(SPI_DISP_FREQ);
    u8g2.begin();

    analogWrite(LED_G_PIN, 5);
    analogWrite(LED_B_PIN, 2);

    SETTINGS.start();

    POWER.onShutdownSubscribe(clearDisplay);
    POWER.onShutdownSubscribe(saveRuntime);
    POWER.start();

    UI.setDisplay(&u8g2);

    UI.start();
    ROOT_UI.start();

    ROOT_UI.setScreen((Screen *)&STATUS_SCREEN);
    CONTROLS.start(5000);

       
    RADIO.setRfSwitchPins(RADIO_RX_EN_PIN, RADIO_TX_EN_PIN);

    RADIO.setDio0Action(radioInterrupt);

    RADIOTASK.start();

}

   

void loop(void) {
    uint32_t delayTime = EXECUTOR.runSchedule();

    if (delayTime > 100000)
        delayTime = 100000;

    if (delayTime > MIN_MICRO_REST) {
        delayMicroseconds(delayTime - MIN_MICRO_REST);
    }
}