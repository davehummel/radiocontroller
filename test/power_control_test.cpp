#include <Arduino.h>
#include <SPI.h>

#include "FDOS_LOG.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include <U8g2lib.h>

U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

Logger FDOS_LOG(&Serial);

VMExecutor executor;

class PowerControlTestTask : RunnableTask {

    uint8_t countdown = 0;

    void run(TIME_INT_t time) {
        digitalWrite(ENABLE_BATTERY_SENSE_PIN, HIGH);
        u8g2.setDrawColor(1);
        u8g2.drawBox(10, 100, 80, 70);
        u8g2.setDrawColor(0);
        u8g2.setCursor(10, 100);
        u8g2.print(countdown);
        u8g2.setCursor(10, 116);
        u8g2.print(analogRead(POWER_BUTTON_PRESS_PIN));
        u8g2.setCursor(10, 132);
        u8g2.print(analogRead(BATTERY_SENSE_PIN));
        digitalWrite(ENABLE_BATTERY_SENSE_PIN, LOW);
        u8g2.sendBuffer();
        if (analogRead(POWER_BUTTON_PRESS_PIN) > 100) {
            countdown = 101;
        }
        if (countdown == 1) {
            digitalWrite(HOLD_POWER_ENABLE_PIN, false);
        }
        if (countdown != 0)
            countdown--;
    }

} powerTask;

void setup(void) {
    pinMode(0,OUTPUT);
    analogWrite(0,100);
    delay(10000);

    pinMode(HOLD_POWER_ENABLE_PIN, OUTPUT);
    digitalWrite(HOLD_POWER_ENABLE_PIN, true);
    pinMode(POWER_BUTTON_PRESS_PIN, INPUT_PULLDOWN);

    pinMode(ENABLE_BATTERY_SENSE_PIN, OUTPUT);
    digitalWrite(ENABLE_BATTERY_SENSE_PIN, LOW);

    pinMode(BATTERY_SENSE_PIN, INPUT_PULLDOWN);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, true);

    // SPI.setSCK(SPI_CLK_PIN);
    SPI.begin();
    // PowerEnable pin must be enabled ASAP to keep system on after power is
    // released
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, true);

    Serial.begin(921600);

    Serial.println("Display Starting ...");
    u8g2.setBusClock(SPI_DISP_FREQ);
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 240, 400);
    u8g2.setDrawColor(0); // in order to make content visable
    u8g2.setFont(u8g2_font_timB14_tr);
    // u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
    // u8g2.setFontDirection(0);

    u8g2.drawStr(0, 0, "Display Ready!");
    u8g2.sendBuffer();

    digitalWrite(LED_PIN, false);

    executor.schedule((RunnableTask *)&powerTask, executor.getTimingPair(1000, FrequencyUnitEnum::milli));
}

void loop(void) {
    uint32_t delayTime = executor.runSchedule();
    // You may want to allow the loop to finish and avoid long delays
    //    if you are using background arduino features
    if (delayTime > 100000)
        delayTime = 100000;
    if (delayTime > MIN_MICRO_REST) {
        delayMicroseconds(delayTime - MIN_MICRO_REST);
    }
}
