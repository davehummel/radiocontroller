#ifndef input__controls_H__
#define input__controls_H__
#include "VMTime.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <Encoder.h>

typedef void (*FunctionPointer)();

class PhysicalInput {
  public:
    PhysicalInput() {}
    bool update();
    bool hasChanged() { return changed; }

    void subscribe(FunctionPointer func);

    void unsubscribe(FunctionPointer func);

    virtual ~PhysicalInput(){}

  protected:
    virtual bool innerUpdate() = 0;

  private:
    const static uint8_t MAX_LISTENERS = 10;
    bool changed;
    FunctionPointer listeners[MAX_LISTENERS] = {0};
};

class JoyInput : public PhysicalInput {
  public:
    JoyInput(int _pinId) : pinId(_pinId) {
        analogReadResolution(ANALOG_READ_RESOLUTION);
        analogReadAveraging(ANALOG_READ_AVERAGING);
        pinMode(pinId, INPUT);
        setHalfRange(127);
    }

    uint16_t min = 1535;       // Minimum raw value expected - For any x <= min then output is 0 (-halfRange if signed).
    uint16_t max = 2535;       // Maximum raw value expected - For any x >= max then output is 2*halfRange (+halfRange if signed).
    uint16_t midMinExc = 2015; // Raw input values less than midMinExc map from 0 to halfRange-1 (-halfRange to -1 if signed). Values between midMinExc and
                               // midMaxExc = halfRange (0 signed)
    uint16_t midMaxExc = 2055; // Raw input values greater than midMaxExc to max map from halfRange+1 to 2*halfRange
                               //(1 to halfRange signed). Values between midMinExc and midMaxExc = halfRange (0 signed)
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

    bool update();

    void addInput(PhysicalInput &input, uint8_t inputId);

    const uint8_t inputCount;

  private:
    PhysicalInput **inputs;
};

#endif