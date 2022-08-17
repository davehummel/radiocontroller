#include <Arduino.h>
#include <SPI.h>

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

Logger FDOS_LOG(&Serial);

U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

void clearDisplay() {
    UI.getDisplay()->clear();
    UI.requestDraw(true);
}

void saveRuntime() { SETTINGS.saveRuntimeSeconds(); }

void setup(void) {
    Serial.begin(921600);
    SPI.begin();
    u8g2.setBusClock(SPI_DISP_FREQ);
    u8g2.begin();

    CONTROLS.update();

    analogWrite(LED_G_PIN, 5);
    analogWrite(LED_B_PIN, 2);

    SETTINGS.start();

    POWER.onShutdownSubscribe(clearDisplay);
    POWER.onShutdownSubscribe(saveRuntime);
     POWER.start();

    UI.setDisplay(&u8g2);
    CONTROLS.update();

    UI.start();
    ROOT_UI.start();
    ROOT_UI.setScreen((Screen *)&STATUS_SCREEN);
    CONTROLS.start(10000);
}

void loop(void) {
    uint32_t delayTime = EXECUTOR.runSchedule();

    if (delayTime > 100000)
        delayTime = 100000;

    if (delayTime > MIN_MICRO_REST) {
        delayMicroseconds(delayTime - MIN_MICRO_REST);
    }
}