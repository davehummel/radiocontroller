#ifndef input__controls_H__
#define input__controls_H__
#include "VMTime.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <Encoder.h>
#include <SettingsStore.h>

typedef void (*FunctionPointer)();

class PhysicalInput {
  public:
    PhysicalInput() {}
    bool update(bool mute);
    bool hasChanged() { return changed; }

    void subscribe(FunctionPointer func);

    void unsubscribe(FunctionPointer func);

    virtual ~PhysicalInput() {}

  protected:
    virtual bool innerUpdate() = 0;

  private:
    const static uint8_t MAX_LISTENERS = 10;
    bool changed;
    FunctionPointer listeners[MAX_LISTENERS] = {0};
};

class JoyInput : public PhysicalInput {

  private:
    EEPROMField::generic_val_t &min, &mid1, &mid2, &max;

  public:
    JoyInput(int _pinId, EEPROMField &minField, EEPROMField &mid1Field, EEPROMField &mid2Field, EEPROMField &maxField)
        : min(minField.getValue()), mid1(mid1Field.getValue()), mid2(mid2Field.getValue()), max(maxField.getValue()), pinId(_pinId) {
        analogReadResolution(ANALOG_READ_RESOLUTION);
        analogReadAveraging(ANALOG_READ_AVERAGING);
        pinMode(pinId, INPUT);
        setHalfRange(127);
    }

    bool inverted = false;

    void setHalfRange(uint16_t _halfRange) {
        halfRange = _halfRange;
        fullRange = halfRange * 2;
    }

    int16_t getSignedValue();    // Returns a value from -halfRange to halfRange using mapping
    uint16_t getUnsignedValue(); // Returns a value from 0 to halfRange*2 using mapping
    uint16_t getRawValue();      // Original analog input

  protected:
    bool innerUpdate();

  private:
    uint16_t rawValue = 0;
    uint16_t unsignedValue = 0;
    const int pinId;

    uint16_t halfRange;
    uint16_t fullRange;
};

class WheelInput : public PhysicalInput {
  public:
    WheelInput(int encAPinID, int encBPinID) : encoder(encAPinID, encBPinID) {}

    int32_t getDelta(); // Original analog input

  protected:
    bool innerUpdate();

  private:
    Encoder encoder;
    int32_t value = 0;
};

class ButtonInput : public PhysicalInput {
  public:
    ButtonInput(int _pinId, int _ledPinID = -1) {
        button.attach(_pinId, INPUT_PULLUP);
        button.setPressedState(LOW);
        ledPinID = _ledPinID;
        if (ledPinID > -1) {
            pinMode(ledPinID, OUTPUT);
            digitalWrite(ledPinID, 0);
        }
    }

    bool isPressed(); // State

    bool wasPressed();
    bool wasReleased();

    uint16_t getLEDValue() { return ledVal; }

    void setLEDValue(uint16_t val);

  protected:
    bool innerUpdate();

  private:
    Button button;
    int ledPinID;
    uint16_t ledVal;
};

class MultiVButton : public PhysicalInput {
  public:
    MultiVButton(int _pinId, uint8_t _states, uint16_t _activationDistance)
        : pinId(_pinId), states(_states), values(new uint16_t[states]), activationDistance(_activationDistance) {
        analogReadResolution(ANALOG_READ_RESOLUTION);
        analogReadAveraging(ANALOG_READ_AVERAGING);
        pinMode(pinId, INPUT_PULLDOWN);
    }

    ~MultiVButton() { delete values; }

    void setStateValue(uint8_t stateNum, uint16_t value) { values[stateNum - 1] = value; }

    uint8_t getState();
    uint16_t getRawValue();

  protected:
    bool innerUpdate();

  private:
    uint16_t rawValue = 0;
    uint8_t currentState = 0;

    const int pinId;
    const uint8_t states;
    uint16_t *values;
    const uint16_t activationDistance;
};

class InputSet {
  public:
    InputSet(uint8_t _inputCount) : inputCount(_inputCount), inputs(new PhysicalInput *[inputCount] { 0 }) {}
    ~InputSet() { delete inputs; }

    bool update(TIME_INT_t time, bool mute);

    void addInput(PhysicalInput &input, uint8_t inputId);

    const uint8_t inputCount;

    TIME_INT_t lastInputMicros(bool reset = false) {
        if (reset)
            inputTimeMicros = microsSinceEpoch();
        return inputTimeMicros;
    }

  private:
    PhysicalInput **inputs;
    TIME_INT_t inputTimeMicros = 0;
};

#endif