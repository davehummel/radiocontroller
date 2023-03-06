
#include <FDOS_LOG.h>
#include <SettingsStore.h>

EEPROMField field_AutoOff_Min(1, 0ul, 240ul, 10ul);
EEPROMField field_AutoOff_V(3.0, 3.6, 3.2);
EEPROMField field_joy2H_Min(2, 1000ul, 2000ul, 1550ul);
EEPROMField field_joy2H_Mid1(2, 1000ul, 3000, 2010ul);
EEPROMField field_joy2H_Mid2(2, 1000ul, 3000, 2080ul);
EEPROMField field_joy2H_Max(2, 2000ul, 3000ul, 2500ul);
EEPROMField field_joy2V_Min(2, 1000ul, 2000ul, 1550ul);
EEPROMField field_joy2V_Mid1(2, 1000ul, 3000ul, 2010ul);
EEPROMField field_joy2V_Mid2(2, 1000ul, 3000ul, 2080ul);
EEPROMField field_joy2V_Max(2, 2000ul, 3000ul, 2500ul);
EEPROMField field_joy1H_Min(2, 1000ul, 2000ul, 1550ul);
EEPROMField field_joy1H_Mid1(2, 1000ul, 3000ul, 2010ul);
EEPROMField field_joy1H_Mid2(2, 1000ul, 3000ul, 2080ul);
EEPROMField field_joy1H_Max(2, 2000ul, 3000ul, 2500ul);
EEPROMField field_joy1V_Min(2, 1000ul, 2000ul, 1550ul);
EEPROMField field_joy1V_Mid1(2, 1000ul, 3000ul, 2010ul);
EEPROMField field_joy1V_Mid2(2, 1000ul, 3000ul, 2080ul);
EEPROMField field_joy1V_Max(2, 2000ul, 3000ul, 2500ul);

EEPROMField field_radio_Freq(900.0, 931.0, 915.0);
EEPROMField field_radio_Linkbw(1, 0ul, 2ul, 2ul);
EEPROMField field_radio_SpreadingFactor(1, 6ul, 12ul, 7ul);
EEPROMField field_radio_CodingRate(1, 5ul, 8ul, 5ul);
EEPROMField field_radio_Power(1, 2ul, 20ul, 10);

EEPROMField field_PID_yaw_kp(0.0f, 2.0, .1, 3);
EEPROMField field_PID_yaw_ki(0.0f, 2.0, .1, 3);
EEPROMField field_PID_yaw_kd(0.0f, 2.0, .1, 3);
EEPROMField field_PID_yaw_max_i(2, 0ul, 2000ul, 100ul);

EEPROMField field_PID_roll_kp(0.0f, 2.0, .1, 3);
EEPROMField field_PID_roll_ki(0.0f, 2.0, .1, 3);
EEPROMField field_PID_roll_kd(0.0f, 2.0, .1, 3);
EEPROMField field_PID_roll_max_i(2, 0ul, 2000ul, 100ul);

EEPROMField field_PID_pitch_kp(0.0f, 2.0, .1, 3);
EEPROMField field_PID_pitch_ki(0.0f, 2.0, .1, 3);
EEPROMField field_PID_pitch_kd(0.0f, 2.0, .1, 3);
EEPROMField field_PID_pitch_max_i(2, 0ul, 2000ul, 100ul);

const uint8_t EEFIELD_COUNT = 35;
EEPROMField *EEPROM_FIELDS[EEFIELD_COUNT] = {
    &field_AutoOff_Min,      &field_AutoOff_V,       &field_joy2H_Min,
    &field_joy2H_Mid1,       &field_joy2H_Mid2,      &field_joy2H_Max,
    &field_joy2V_Min,        &field_joy2V_Mid1,      &field_joy2V_Mid2,
    &field_joy2V_Max,        &field_joy1H_Min,       &field_joy1H_Mid1,
    &field_joy1H_Mid2,       &field_joy1H_Max,       &field_joy1V_Min,
    &field_joy1V_Mid1,       &field_joy1V_Mid2,      &field_joy1V_Max,
    &field_radio_Freq,       &field_radio_Linkbw,    &field_radio_SpreadingFactor,
    &field_radio_CodingRate, &field_radio_Power,     &field_PID_yaw_kp,
    &field_PID_yaw_ki,       &field_PID_yaw_kd,      &field_PID_yaw_max_i,
    &field_PID_roll_kp,      &field_PID_roll_ki,     &field_PID_roll_kd,
    &field_PID_roll_max_i,   &field_PID_pitch_kp,    &field_PID_pitch_ki,
    &field_PID_pitch_kd,     &field_PID_pitch_max_i,
};

Settings::Settings() {
    uint16_t addr = FIRST_SETTING_ADDR;
    for (uint8_t i = 0; i < EEFIELD_COUNT; i++) {
        Serial.print(addr);
        EEPROM_FIELDS[i]->setAddr(addr);
        Serial.print(" = ");
        Serial.println(EEPROM_FIELDS[i]->getText());
        addr += EEPROM_FIELDS[i]->getSize();
    }
}

void Settings::initializeEEPROM() {
    tone(SND_OUTPUT_PIN, 400);
    delay(50);
    noTone(SND_OUTPUT_PIN);
    delay(200);
    tone(SND_OUTPUT_PIN, 600);
    delay(50);
    noTone(SND_OUTPUT_PIN);
    delay(200);
    tone(SND_OUTPUT_PIN, 800);
    delay(50);
    noTone(SND_OUTPUT_PIN);
    delay(200);
    tone(SND_OUTPUT_PIN, 1000);
    delay(50);
    noTone(SND_OUTPUT_PIN);
    delay(200);

    EEPROM.put(INIT_VAL_SIZE, (uint32_t)0);
    EEPROM.put(0, INIT_VAL);

    for (uint8_t i = 0; i < EEFIELD_COUNT; i++) {
        EEPROM_FIELDS[i]->overwriteWithDefault();
    }
}

void Settings::start() {

    uint16_t initVal = EEPROM.get(0, initVal);

    bool override = (!digitalRead(BTN1_PRESS_PIN) && !digitalRead(BTN2_PRESS_PIN));
    if (override) {
        tone(SND_OUTPUT_PIN, 1200);
        delay(200);
        noTone(SND_OUTPUT_PIN);
        override = (!digitalRead(BTN1_PRESS_PIN) && !digitalRead(BTN2_PRESS_PIN));
        delay(50);
        if (override) {
            override = (!digitalRead(BTN1_PRESS_PIN) && !digitalRead(BTN2_PRESS_PIN));
            if (override) {
                tone(SND_OUTPUT_PIN, 1400);
                delay(200);
                noTone(SND_OUTPUT_PIN);
                override = (!digitalRead(BTN1_PRESS_PIN) && !digitalRead(BTN2_PRESS_PIN));
            }
        }
    }
    if ((initVal != INIT_VAL) || override) {
        initializeEEPROM();
    }
}

uint32_t Settings::getRuntimeSeconds() {
    if (runtimeSeconds == 0xFFFF) {
        EEPROM.get(INIT_VAL_SIZE, runtimeSeconds);
    }
    return micros64() / 1000000 + runtimeSeconds;
}

void Settings::saveRuntimeSeconds() { EEPROM.put(INIT_VAL_SIZE, getRuntimeSeconds()); }

char EEPROMField::buffer[32] = {0};

void EEPROMField::modify(int delta) {
    if (!cached) {
        readEEPROM();
    }
    modified = true;
    switch (type) {
    case U8:
    case U16:
    case U32:
        rawVal.u32 += delta * decScale;
        if (rawVal.u32 > rawMaxInc.u32) {
            rawVal.u32 = rawMinInc.u32;
        } else if (rawVal.u32 < rawMinInc.u32) {
            rawVal.u32 = rawMaxInc.u32;
        }
        break;
    case I8:
    case I16:
    case I32:
    case I64:
        rawVal.i64 += delta * decScale;
        if (rawVal.i64 > rawMaxInc.i64) {
            rawVal.i64 = rawMinInc.i64;
        } else if (rawVal.i64 < rawMinInc.i64) {
            rawVal.i64 = rawMaxInc.i64;
        }
        break;
    case F:
        rawVal.f += delta * decScale;
        if (rawVal.f > rawMaxInc.f) {
            rawVal.f = rawMinInc.f;
        } else if (rawVal.f < rawMinInc.f) {
            rawVal.f = rawMaxInc.f;
        }
        break;
    }
}

EEPROMField::EEPROMField(uint8_t byteSize, int64_t min, int64_t max, int64_t def, uint8_t decimalScale)
    : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(byteSize == 1   ? I8
                                                                     : byteSize == 2 ? I16
                                                                     : byteSize == 4 ? I32
                                                                                     : I64),
      decScale(pow10(decimalScale)) {}

EEPROMField::EEPROMField(uint8_t byteSize, uint32_t min, uint32_t max, uint32_t def, uint8_t decimalScale)
    : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(byteSize == 1   ? U8
                                                                     : byteSize == 2 ? U16
                                                                                     : I32),
      decScale(pow10(decimalScale)) {}

EEPROMField::EEPROMField(float min, float max, float def, uint8_t decimalScale)
    : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(F), decScale(pow10(-decimalScale)) {}

void EEPROMField::save() {
    if (!modified)
        return;
    saveEEPROM();
}

void EEPROMField::reset() {
    modified = false;
    readEEPROM();
}

void EEPROMField::overwriteWithDefault() {
    rawVal = defVal;
    saveEEPROM();
}

const char *EEPROMField::getText() {
    if (!cached) {
        readEEPROM();
    }
    switch (type) {
    case U8:
    case U16:
    case U32:
        sprintf(EEPROMField::buffer, "%" PRIu32 "", rawVal.u32);
        break;
    case I8:
    case I16:
    case I32:
    case I64:
        sprintf(EEPROMField::buffer, "%" PRId64 "", rawVal.i64);
        break;
    case F:
        if (decScale == .1)
            sprintf(EEPROMField::buffer, "%.1f", rawVal.f);
        else if (decScale == .01)
            sprintf(EEPROMField::buffer, "%.2f", rawVal.f);
        else if (decScale == .001)
            sprintf(EEPROMField::buffer, "%.3f", rawVal.f);

        break;
    }
    return EEPROMField::buffer;
}

uint8_t EEPROMField::getSize() {
    switch (type) {
    case I8:
    case U8:
        return 1;
    case I16:
    case U16:
        return 2;
    case I32:
    case U32:
    case F:
        return 4;
    case I64:
        return 8;
    }
    return 0;
}

void EEPROMField::readEEPROM() {
    cached = true;
    FDOS_LOG.printf("%p reading from addr: %i\n", this, addr);
    switch (type) {
    case I8:
        EEPROM.get(addr, rawVal.i8);
        return;
    case U8:
        EEPROM.get(addr, rawVal.u8);
        return;
    case I16:
        EEPROM.get(addr, rawVal.i16);
        return;
    case U16:
        EEPROM.get(addr, rawVal.u16);
        return;
    case I32:
        EEPROM.get(addr, rawVal.i32);
        return;
    case U32:
        EEPROM.get(addr, rawVal.u32);
        return;
    case F:
        EEPROM.get(addr, rawVal.f);
        return;
    case I64:
        EEPROM.get(addr, rawVal.i64);
        return;
    }
}

void EEPROMField::saveEEPROM() {
    FDOS_LOG.printf("%p saving %s at addr: %i\n", this, getText(), addr);
    switch (type) {
    case I8:
        EEPROM.put(addr, rawVal.i8);
        return;
    case U8:
        EEPROM.put(addr, rawVal.u8);
        return;
    case I16:
        EEPROM.put(addr, rawVal.i16);
        return;
    case U16:
        EEPROM.put(addr, rawVal.u16);
        return;
    case I32:
        EEPROM.put(addr, rawVal.i32);
        return;
    case U32:
        EEPROM.put(addr, rawVal.u32);
        return;
    case F:
        EEPROM.put(addr, rawVal.f);
        return;
    case I64:
        EEPROM.put(addr, rawVal.i64);
        return;
    }
}

Settings SETTINGS;
