#include <InputControls.h>

bool PhysicalInput::update(){
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
        unsignedValue = 254;
    } else if (rawValue < midMinExc) {
        unsignedValue = ((rawValue - min) * 126ul) / (midMinExc - min);
    } else if (rawValue > midMaxExc) {
        unsignedValue = 127 + ((rawValue - midMinExc) * 126ul) / (max - midMinExc);
    } else {
        unsignedValue = 127;
    }
    return unsignedValue != prevUnsignedValue;
}

int8_t JoyInput::getSignedValue() { return unsignedValue - 127; }

uint8_t JoyInput::getUnsignedValue() { return unsignedValue; }

uint16_t JoyInput::getRawValue() { return rawValue; }

bool WheelInput::innerUpdate() {
    value = encoder.readAndReset();
    return value != 0;
}

int32_t WheelInput::getDelta() {
    return value;
}

bool ButtonInput::innerUpdate(){
    return button.update();
}

bool ButtonInput::isPressed(){
    return button.isPressed();
}

bool ButtonInput::wasPressed(){
    return button.pressed();
}

bool ButtonInput::wasReleased(){
    return button.released();
}

bool InputSet::update(){
    bool anyUpdates = false;
    mask = 0;
    for (uint8_t i = 0; i < inputCount ; i++){
        if (inputs[i] == NULL)
            continue;
        bool updated=inputs[i]->update();
        anyUpdates|=updated;
        mask|=updated<<i;
    }
    return anyUpdates;
}

void InputSet::addInput(PhysicalInput* input){
    for (uint8_t i = 0 ; i < inputCount ;i++){
        if (inputs[i] == NULL){
            inputs[i] = input;
            break;
        }
    }
}
