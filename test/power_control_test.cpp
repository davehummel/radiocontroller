#include <Arduino.h>
#include <SPI.h>

#include "FDOS_LOG.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include <U8g2lib.h>

U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

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
        Serial.print(countdown);
        Serial.print(":");
        u8g2.setCursor(10, 116);
        u8g2.print(analogRead(POWER_BUTTON_SENSE_PIN));
        u8g2.setCursor(10, 132);
        u8g2.print(analogRead(BATTERY_SENSE_PIN));
        Serial.println(analogRead(BATTERY_SENSE_PIN));
        digitalWrite(ENABLE_BATTERY_SENSE_PIN, LOW);

        u8g2.sendBuffer();
        if (analogRead(POWER_BUTTON_SENSE_PIN) > 100) {
            countdown = 101;
        }
        if (countdown == 1) {
            digitalWrite(HOLD_POWER_ENABLE_PIN, false);
        }
        // analogWrite(LED_R_PIN,(countdown%8) *31);
        // analogWrite(LED_G_PIN,(countdown%16) *15);
        digitalWrite(BTN1_LED_PIN, countdown % 3 == 0);
        digitalWrite(BTN2_LED_PIN, countdown % 4 == 0);
        digitalWrite(BTN3_LED_PIN, countdown % 5 == 0);
        digitalWrite(BTN4_LED_PIN, countdown % 6 == 0);
        digitalWrite(BTN5_LED_PIN, countdown % 7 == 0);

        digitalWrite(DISPLAY_LIGHT_PIN, !digitalRead(BTN1_PRESS_PIN));
        digitalWrite(LED_R_PIN, !digitalRead(BTN5_PRESS_PIN));
        digitalWrite(LED_G_PIN, !digitalRead(BTN4_PRESS_PIN));
        digitalWrite(LED_B_PIN, !digitalRead(BTN3_PRESS_PIN));

        if (countdown != 0)
            countdown--;
    }

} powerTask;

void setup(void) {

    pinMode(HOLD_POWER_ENABLE_PIN, OUTPUT);
    digitalWrite(HOLD_POWER_ENABLE_PIN, true);
    pinMode(POWER_BUTTON_SENSE_PIN, INPUT_PULLDOWN);

    pinMode(ENABLE_BATTERY_SENSE_PIN, OUTPUT);
    digitalWrite(ENABLE_BATTERY_SENSE_PIN, LOW);

    pinMode(BATTERY_SENSE_PIN, INPUT_PULLDOWN);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, true);

    pinMode(BTN1_PRESS_PIN, INPUT_PULLUP);
    pinMode(BTN2_PRESS_PIN, INPUT_PULLUP);
    pinMode(BTN3_PRESS_PIN, INPUT_PULLUP);
    pinMode(BTN4_PRESS_PIN, INPUT_PULLUP);
    pinMode(BTN5_PRESS_PIN, INPUT_PULLUP);

    pinMode(BTN1_LED_PIN, OUTPUT);
    pinMode(BTN2_LED_PIN, OUTPUT);
    pinMode(BTN3_LED_PIN, OUTPUT);
    pinMode(BTN4_LED_PIN, OUTPUT);
    pinMode(BTN5_LED_PIN, OUTPUT);

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
    u8g2.drawBox(0, 0, 400, 240);
    u8g2.setDrawColor(0); // in order to make content visable
    u8g2.setFont(u8g2_font_timB14_tr);
    // u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
    // u8g2.setFontDirection(0);

    u8g2.drawStr(1, 1, "Display Ready!");
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
