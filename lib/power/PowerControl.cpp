#include "PowerControl.h"
#include "SettingsStore.h"

Power POWER(HOLD_POWER_ENABLE_PIN, POWER_BUTTON_SENSE_PIN, BATTERY_SENSE_PIN, ENABLE_BATTERY_SENSE_PIN);

Power::Power(int hold_power_pin, int power_btn_sense_pin, int bat_sense_pin, int en_bat_sense_pin)
    : holdPowerPin(hold_power_pin), powerBtnSensePin(power_btn_sense_pin), batSensePin(bat_sense_pin), enBatSensePin(en_bat_sense_pin) {
    pinMode(holdPowerPin, OUTPUT);
    digitalWrite(holdPowerPin, true);
    analogReadResolution(ANALOG_READ_RESOLUTION);
    analogReadAveraging(ANALOG_READ_AVERAGING);
    pinMode(powerBtnSensePin, INPUT_PULLDOWN);

    pinMode(enBatSensePin, OUTPUT);
    digitalWrite(enBatSensePin, LOW);

    pinMode(batSensePin, INPUT_PULLDOWN);

    digitalWrite(LED_PIN, true);
}

void Power::start() { EXECUTOR.schedule(this, EXECUTOR.getTimingPair(300, FrequencyUnitEnum::milli)); }

void Power::powerDown() {
    digitalWrite(LED_G_PIN, true);
    digitalWrite(LED_R_PIN, true);
    for (uint8_t i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != NULL) {
            listeners[i]();
        }
    }
    tone(SND_OUTPUT_PIN, 200, 50);
    digitalWrite(LED_G_PIN, false);
    digitalWrite(holdPowerPin, false);
    delay(10000);
    isPoweredDown = true;
}

void Power::onShutdownSubscribe(FunctionPointer listener) {
    for (uint8_t i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] == NULL) {
            listeners[i] = listener;
            return;
        }
    }
}

void Power::onShutdownUnsubscribe(FunctionPointer listener) {
    bool found = false;
    for (uint8_t i = 0; i < MAX_LISTENERS; i++) {
        if (found) {
            listeners[i - 1] = listeners[i];
        } else if (listeners[i] == listener) {
            listeners[i] = NULL;
            found = true;
        }
    }
}

float Power::getBatteryVoltage() {
    digitalWrite(enBatSensePin, HIGH);
    uint16_t raw = analogRead(batSensePin);

    return raw / BATTERY_VLT_FACTOR;
}

uint8_t Power::getBatteryPercent() {
    float vlt = getBatteryVoltage();
    if (vlt <= field_AutoOff_V.getValue().f)
        return 0;
    if (vlt >= 4.2)
        return 100;
    return (vlt - field_AutoOff_V.getValue().f) * 100 / (4.2 - field_AutoOff_V.getValue().f);
}

bool Power::isPowerBtnPressed() { return analogRead(powerBtnSensePin) > 100; }

void Power::run(TIME_INT_t time) {

    if (isPoweredDown) {
        digitalWrite(LED_R_PIN, true);
        tone(SND_OUTPUT_PIN, 500);
        return;
    }

    if (!initialBtnRelease) {
        if (isPowerBtnPressed())
            return;
        initialBtnRelease = true;
    } else {
        if (isPowerBtnPressed()) {
            if (!firstBtnOffDetected) {
                firstBtnOffDetected = true;
            } else {
                powerDown();
                return;
            }
        } else {
            firstBtnOffDetected = false;
        }
    }

    if (getBatteryVoltage() <= field_AutoOff_V.getValue().f) {
        for (uint8_t i = 0; i < 30; i++) {
            delay(50);
            analogWrite(LED_R_PIN, i);
        }
        powerDown();
        return;
    }
}