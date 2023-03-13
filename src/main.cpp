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
#include <StatusScreen.h>

#include <ControllerRadio.h>
#include <RadioTask.h>

Logger FDOS_LOG(&Serial);

void saveRuntime() { SETTINGS.saveRuntimeSeconds(); }

U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

void clearDisplay() {
    UI.getDisplay()->clear();
    UI.requestDraw(true);
}
SX1276 RADIO = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN, SPI, SPISettings(SPI_RADIO_FREQ, MSBFIRST, SPI_MODE0));

RadioTask RADIOTASK(&RADIO);

void radioInterrupt(void) { RADIOTASK.interruptTriggered(); }

void setup(void) {
    Serial.begin(921600);
    SPI.begin();
    u8g2.setBusClock(SPI_DISP_FREQ);
    u8g2.begin();

    analogWrite(LED_R_PIN, 3);
    analogWrite(LED_B_PIN, 4);

    SETTINGS.start();

    POWER.onShutdownSubscribe(clearDisplay);
    POWER.onShutdownSubscribe(saveRuntime);
    POWER.start();

    UI.setDisplay(&u8g2);

    UI.start();
    ROOT_UI.start();

    ROOT_UI.setScreen((Screen *)&STATUS_SCREEN);
    CONTROLS.start(10000);

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