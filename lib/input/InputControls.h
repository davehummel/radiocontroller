#ifndef input__controls_H__
#define input__controls_H__
#include "VMTime.h"
#include <Arduino.h>
#include <Bounce2.h>
#include <Encoder.h>

class PhysicalInput {
  public:
    PhysicalInput() {}
    bool update();
    bool hasChanged() { return changed; }

    void setOnChangeFunction(void (* func)()) {onChangeFunc = func;};

  protected:
    virtual bool innerUpdate() = 0;
private:
    bool changed;
    void (* onChangeFunc)() = NULL;

};

class JoyInput : public PhysicalInput {
  public:
    JoyInput(int _pinId) : pinId(_pinId) {}

    uint16_t min = 0;       // Minimum raw value expected - For any x <= min then output is 0 (-127 signed).
    uint16_t max = 254;     // Maximum raw value expected - For any x >= max then output is 254 (127 signed).
    uint16_t midMinExc = 0; // Values less than midMinExc map from 0 to 126 (-127 to -1 signed). Values between midMinExc and midMaxExc = 127 (0 signed)
    uint16_t midMaxExc = 0; // Values greater than midMaxExc to max map from 128 to 254
                            //(1 to 127 signed). Values between midMinExc and midMaxExc = 127 (0 signed)

    int8_t getSignedValue();    // Returns a value from -127 to 127 with errors as -128
    uint8_t getUnsignedValue(); // Returns a value from 0 to 254 with errors as 255
    uint16_t getRawValue();     // Original analog input

  protected:
    bool innerUpdate();

  private:
    uint16_t rawValue = 0;
    uint8_t unsignedValue = 0;
    const int pinId;
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
    ButtonInput(int _pinId) {
        button.attach(_pinId, INPUT_PULLUP);
        button.setPressedState(LOW);
    }

    bool isPressed(); // State

    bool wasPressed();
    bool wasReleased();

  protected:
    bool innerUpdate();

  private:
    Button button;
};

class InputSet {
  public:
    InputSet(uint8_t _inputCount) : inputCount(_inputCount), inputs(new PhysicalInput *[inputCount] { 0 }) {}
    ~InputSet() { delete inputs; }

    bool update();

    void addInput(PhysicalInput *input);

    bool hasChanged(uint8_t inputId);

    const uint8_t inputCount;

  private:
    PhysicalInput **inputs;

    uint64_t mask;
};

#endif