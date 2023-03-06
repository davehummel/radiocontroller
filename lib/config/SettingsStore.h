#ifndef settings__store_H__
#define settings__store_H__
#include <Arduino.h>
#include <EEPROM.h>
#include "VMTime.h"

class Settings {
  private:
    const uint16_t INIT_VAL = 0xAD;
    const static uint16_t INIT_VAL_SIZE = 2, RUNTIME_SECONDS_SIZE = 4;

    const static uint16_t FIRST_SETTING_ADDR = INIT_VAL_SIZE + RUNTIME_SECONDS_SIZE;

    uint32_t runtimeSeconds = 0xFFFF;

    uint8_t autoOffMinutes = 0XFF;

    void initializeEEPROM();

  public:
    Settings();

    void start();

    void saveRuntimeSeconds();

    uint32_t getRuntimeSeconds();

    friend class SettingField;
};

class SettingField {

  public:
    virtual bool readOnly() { return true; };

    virtual void modify(int delta){};

    virtual void save(){};

    virtual void reset(){};

    virtual void overwriteWithDefault(){};

    virtual const char *getText() = 0;
};

class LabelField : public SettingField {
  protected:
    String text;

  public:
    LabelField(const char *initVal) { text = initVal; }
    bool readOnly() { return true; };

    void modify(int delta){};

    void save(){};

    void reset(){};

    const char *getText() { return text.c_str(); }
};

class EEPROMField : public SettingField {
  public:
    union generic_val_t {
        generic_val_t(int64_t v) : i64(v){};
        generic_val_t(uint32_t v) : u32(v){};
        generic_val_t(float v) : f(v){};
        uint8_t u8;
        int8_t i8;
        uint16_t u16;
        int16_t i16;
        int32_t i32;
        uint32_t u32;
        int64_t i64;
        float f;
    };

  private:
    enum val_type_t { U8, I8, U16, I16, U32, I32, I64, F };

    uint16_t addr;

    generic_val_t rawVal;
    const generic_val_t rawMinInc;
    const generic_val_t rawMaxInc;
    const generic_val_t defVal;

    const float decScale;

    static char buffer[32];

    bool modified = false;
    bool cached = false;

    void readEEPROM();
    void saveEEPROM();

  protected:
    void setAddr(uint16_t _addr) {
        Serial.printf("%p setting addr internally %i\n",this, _addr);
        addr = _addr;
        cached = false;
    }

  public:
    const val_type_t type;

    EEPROMField(uint8_t byteSize, int64_t min, int64_t max, int64_t def,uint8_t decimalScale = 1);
    EEPROMField(uint8_t byteSize, uint32_t min, uint32_t max, uint32_t def,uint8_t decimalScale = 1);
    EEPROMField(float min, float max, float def,uint8_t decimalScale = 2);

    bool readOnly() { return false; };

    void modify(int delta);

    void save();

    void reset();

    void overwriteWithDefault();

    const char *getText();

    uint8_t getSize();

    generic_val_t &getValue() {
        if (!cached)
            readEEPROM();
        return rawVal;
    }

    friend class Settings;
};

extern EEPROMField field_AutoOff_Min;
extern EEPROMField field_AutoOff_V;
extern EEPROMField field_joy2H_Min;
extern EEPROMField field_joy2H_Mid1;
extern EEPROMField field_joy2H_Mid2;
extern EEPROMField field_joy2H_Max;
extern EEPROMField field_joy2V_Min;
extern EEPROMField field_joy2V_Mid1;
extern EEPROMField field_joy2V_Mid2;
extern EEPROMField field_joy2V_Max;
extern EEPROMField field_joy1H_Min;
extern EEPROMField field_joy1H_Mid1;
extern EEPROMField field_joy1H_Mid2;
extern EEPROMField field_joy1H_Max;
extern EEPROMField field_joy1V_Min;
extern EEPROMField field_joy1V_Mid1;
extern EEPROMField field_joy1V_Mid2;
extern EEPROMField field_joy1V_Max;

extern EEPROMField field_radio_Freq;
extern EEPROMField field_radio_Linkbw; 
extern EEPROMField field_radio_SpreadingFactor;
extern EEPROMField field_radio_CodingRate;
extern EEPROMField field_radio_Power;

extern EEPROMField field_PID_yaw_kp,field_PID_yaw_ki,field_PID_yaw_kd,field_PID_yaw_max_i;
extern EEPROMField field_PID_roll_kp,field_PID_roll_ki,field_PID_roll_kd,field_PID_roll_max_i;
extern EEPROMField field_PID_pitch_kp,field_PID_pitch_ki,field_PID_pitch_kd,field_PID_pitch_max_i;


extern Settings SETTINGS;
#endif