#include <InputControls.h>

bool PhysicalInput::update(bool mute) {
    changed = innerUpdate();
    if (changed){
        if (mute)
            return false;
        for (uint8_t i = 0;i<MAX_LISTENERS;i++){
            if (listeners[i]==NULL)
                break;
            listeners[i]();
        }
    }
    return changed;
}

void PhysicalInput::subscribe(FunctionPointer func) {
    for (uint8_t i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] == NULL) {
            listeners[i] = func;
            return;
        }
    }
}

void PhysicalInput::unsubscribe(FunctionPointer func) {
    bool found = false;
    for (uint8_t i = 0; i < MAX_LISTENERS; i++) {
        if (found) {
            listeners[i - 1] = listeners[i];
        } else if (listeners[i] == func) {
            listeners[i] = NULL;
            found = true;
        }
    }
}

bool JoyInput::innerUpdate() {
    rawValue = analogRead(pinId);
    uint8_t prevUnsignedValue = unsignedValue;
    if (rawValue <= min.u16) {
        unsignedValue = 0;
    } else if (rawValue >= max.u16) {
        unsignedValue = fullRange;
    } else if (rawValue < mid1.u16) {
        unsignedValue = ((rawValue - min.u16) * halfRange) / (mid1.u16 - min.u16);
    } else if (rawValue > mid2.u16) {
        unsignedValue = halfRange + ((rawValue - mid2.u16) * halfRange) / (max.u16 - mid2.u16);
    } else {
        unsignedValue = halfRange;
    }
    if (inverted)
        unsignedValue = fullRange - unsignedValue;
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

void ButtonInput::setLEDValue(uint16_t val) {
    if (ledPinID == -1)
        return;
    analogWrite(ledPinID,val);
    ledVal = val;
}

bool MultiVButton::innerUpdate() {
    rawValue = analogRead(pinId);
    uint8_t newState = 0;
    if (rawValue > activationDistance) {
        for (; newState < states; newState++) {
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

bool InputSet::update(bool mute) {
    bool anyUpdates = false;
    // mask = 0;
    for (uint8_t i = 0; i < inputCount; i++) {
        if (inputs[i] == NULL)
            continue;
        bool updated = inputs[i]->update(mute);
        anyUpdates |= updated;
        // mask |= updated << i;
    }
    return anyUpdates;
}

void InputSet::addInput(PhysicalInput &input, uint8_t inputId) {
    if (inputs[inputId] != NULL) {
        delete inputs[inputId];
    }
    inputs[inputId] = &input;
}
