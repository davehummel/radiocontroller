#include <InputControls.h>

bool PhysicalInput::update() {
    changed = innerUpdate();
    if (changed && onChangeFunc != NULL)
        onChangeFunc();
    return changed;
}

bool JoyInput::innerUpdate() {
    rawValue = analogRead(pinId);
    uint8_t prevUnsignedValue = unsignedValue;
    if (rawValue <= min) {
        unsignedValue = 0;
    } else if (rawValue >= max) {
        unsignedValue = fullRange;
    } else if (rawValue < midMinExc) {
        unsignedValue = ((rawValue - min) * inputMultiplier) / (midMinExc - min);
    } else if (rawValue > midMaxExc) {
        unsignedValue = halfRange + ((rawValue - midMinExc) * inputMultiplier) / (max - midMinExc);
    } else {
        unsignedValue = halfRange;
    }
    return unsignedValue != prevUnsignedValue;
}

int16_t JoyInput::getSignedValue() { return unsignedValue - halfRange; }

uint16_t JoyInput::getUnsignedValue() { return unsignedValue; }

uint16_t JoyInput::getRawValue() { return rawValue; }

bool WheelInput::innerUpdate() {
    value = encoder.readAndReset();
    return value != 0;
}

int32_t WheelInput::getDelta() { return value; }

bool ButtonInput::innerUpdate() { return button.update(); }

bool ButtonInput::isPressed() { return button.isPressed(); }

bool ButtonInput::wasPressed() { return button.pressed(); }

bool ButtonInput::wasReleased() { return button.released(); }

bool MultiVButton::innerUpdate() {
    rawValue = analogRead(pinId);
    uint8_t newState = 0;
    if (rawValue > activationDistance) {
        for (newState; newState < states; newState++) {
            if (abs(values[newState] - rawValue) < activationDistance) {
                newState = newState + 1;
                if (newState != currentState) {
                    currentState = newState;
                    return true;
                } else {
                    return false;
                }
            }
        }
        newState = 0;
    }
    if (newState != currentState) {
        currentState = newState;
        return true;
    } else {
        return false;
    }
}

uint8_t MultiVButton::getState() { return currentState; }

uint16_t MultiVButton::getRawValue() { return rawValue; }

bool InputSet::update() {
    bool anyUpdates = false;
    mask = 0;
    for (uint8_t i = 0; i < inputCount; i++) {
        if (inputs[i] == NULL)
            continue;
        bool updated = inputs[i]->update();
        anyUpdates |= updated;
        mask |= updated << i;
    }
    return anyUpdates;
}

void InputSet::addInput(PhysicalInput *input, uint8_t inputId) {
        if (inputs[inputId]!=NULL){
            delete inputs[inputId];
        }
        inputs[inputId] = input;
}


