#include <EEPROM.h>
#include <FDOS_LOG.h>
#include <SettingsStore.h>

EEPROMField field_AutoOff_Min(1, 0ul, 240ul, 10ul);
EEPROMField field_AutoOff_V(3.0, 3.6, 3.2);

const uint8_t EEFIELD_COUNT = 2;
EEPROMField EEPROM_FIELDS[EEFIELD_COUNT] = {field_AutoOff_Min, field_AutoOff_V};

Settings::Settings() {
    uint16_t addr = FIRST_SETTING_ADDR;
    for (uint8_t i = 0; i < EEFIELD_COUNT; i++) {
        EEPROM_FIELDS[i].setAddr(addr);
        addr += EEPROM_FIELDS[i].getSize();
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
        EEPROM_FIELDS[i].overwriteWithDefault();
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
    FDOS_LOG.printf("EEPROM Delta=%i\n", delta);
    if (!cached) {
        readEEPROM();
    }
    modified = true;
    switch (type) {
    case U8:
    case U16:
    case U32:
        FDOS_LOG.printf("updating U32 val from %i ", rawVal.u32);
        rawVal.u32 += delta;
        if (rawVal.u32 > rawMaxInc.u32) {
            rawVal.u32 = rawMinInc.u32;
        } else if (rawVal.u32 < rawMinInc.u32) {
            rawVal.u32 = rawMaxInc.u32;
        }
        FDOS_LOG.printf(" to %i\n", rawVal.u32, rawVal.u32);
        break;
    case I8:
    case I16:
    case I32:
    case I64:
        FDOS_LOG.printf("updating I64 val from %" PRIi64 " ", rawVal.i64);

        rawVal.i64 += delta;
        if (rawVal.i64 > rawMaxInc.i64) {
            rawVal.i64 = rawMinInc.i64;
        } else if (rawVal.i64 < rawMinInc.i64) {
            rawVal.i64 = rawMaxInc.i64;
        }
        FDOS_LOG.printf("to %" PRIi64 "\n", rawVal.i64);
        break;
    case F:
        FDOS_LOG.printf("updateing F val from %f ", rawVal.f);
        rawVal.f += delta / 100.0;
        if (rawVal.f > rawMaxInc.f) {
            rawVal.f = rawVal.f;
        } else if (rawVal.f < rawMinInc.f) {
            rawVal.f = rawMaxInc.f;
        }
        FDOS_LOG.printf("to %f\n", rawVal.f);
        break;
    }
}

EEPROMField::EEPROMField(uint8_t byteSize, int64_t min, int64_t max, int64_t def)
    : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(byteSize == 1   ? I8
                                                                     : byteSize == 2 ? I16
                                                                     : byteSize == 4 ? I32
                                                                                     : I64) {}

EEPROMField::EEPROMField(uint8_t byteSize, uint32_t min, uint32_t max, uint32_t def)
    : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(byteSize == 1   ? U8
                                                                     : byteSize == 2 ? U16
                                                                                     : I32) {}

EEPROMField::EEPROMField(float min, float max, float def) : rawVal(def), rawMinInc(min), rawMaxInc(max), defVal(def), type(F) {}

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
        sprintf(EEPROMField::buffer, "%0.2f", rawVal.f);

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
