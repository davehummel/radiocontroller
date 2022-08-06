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

    void setOnChangeFunction(void (*func)()) { onChangeFunc = func; };

  protected:
    virtual bool innerUpdate() = 0;

  private:
    bool changed;
    void (*onChangeFunc)() = NULL;
};

class JoyInput : public PhysicalInput {
  public:
    JoyInput(int _pinId) : pinId(_pinId) {
        analogReadResolution(12);
        analogReadAveraging(8);
        pinMode(pinId, INPUT);
        setHalfRange(127);
    }
    
    uint16_t min = 24;       // Minimum raw value expected - For any x <= min then output is 0 (-halfRange if signed).
    uint16_t max = 4072;     // Maximum raw value expected - For any x >= max then output is 2*halfRange (+halfRange if signed).
    uint16_t midMinExc = 2000; // Raw input values less than midMinExc map from 0 to halfRange-1 (-halfRange to -1 if signed). Values between midMinExc and midMaxExc = halfRange (0 signed)
    uint16_t midMaxExc = 2096; // Raw input values greater than midMaxExc to max map from halfRange+1 to 2*halfRange
                            //(1 to halfRange signed). Values between midMinExc and midMaxExc = halfRange (0 signed)

    void setHalfRange(uint16_t _halfRange){
     halfRange=_halfRange;
     fullRange = halfRange*2;
     inputMultiplier = halfRange-1;
    }

    int16_t getSignedValue();    // Returns a value from -halfRange to halfRange using mapping
    uint16_t getUnsignedValue(); // Returns a value from 0 to halfRange*2 using mapping
    uint16_t getRawValue();     // Original analog input

  protected:
    bool innerUpdate();

  private:
    uint16_t rawValue = 0;
    uint8_t unsignedValue = 0;
    const int pinId;
 
     uint16_t halfRange;
     uint16_t fullRange;
     uint32_t inputMultiplier;
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

class MultiVButton : public PhysicalInput {
  public:
    MultiVButton(int _pinId, uint8_t _states, uint16_t _activationDistance)
        : pinId(_pinId), states(_states), values(new uint16_t[states]), activationDistance(_activationDistance) {
        analogReadResolution(12);
        analogReadAveraging(8);
        pinMode(pinId, INPUT_PULLDOWN);
    }

    ~MultiVButton(){
      delete values;
    }

    void setStateValue(uint8_t stateNum, uint16_t value){
      values[stateNum-1] = value;
    }

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
    InputSet(uint8_t _inputCount) : inputCount(_inputCount), inputs(new PhysicalInput *[inputCount] { 0 }) {
        listeners = new FunctionPointer *[inputCount];
        for (uint8_t i = 0; i < inputCount; i++) {
            listeners[i] = new FunctionPointer[8];
        }
    }
    ~InputSet() {
        delete inputs;
        for (uint8_t i = 0; i < inputCount; i++) {
            delete listeners[i];
        }
        delete listeners;
    }

    bool update();

    void addInput(PhysicalInput *input, uint8_t inputId);

    bool hasChanged(uint8_t inputId);

    const uint8_t inputCount;

    void subscribe(FunctionPointer, uint8_t inputId);

    void unsubscribe(FunctionPointer, uint8_t inputId);

  private:
    PhysicalInput **inputs;

    FunctionPointer **listeners;

    uint64_t mask;
};

#endif