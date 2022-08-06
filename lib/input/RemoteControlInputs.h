#ifndef remote__input__controls_H__
#define remote__input__controls_H__
#include "VMTime.h"
#include <Arduino.h>
#include <InputControls.h>

#define INPUT_COUNT 13
enum CONTROLS_ENUM { POWER,BTN1,BTN2,BTN3,BTN4,BTN5,ARROWS,WHEEL,MID_BTN,JOY1_H,JOY1_V,JOY2_H,JOY2_V };

class RemoteControlInputSet:protected InputSet{
    RemoteControlInputSet();

    void attach(CONTROLS_ENUM controlNum,FunctionPointer function); 
    void detach(CONTROLS_ENUM controlNum,FunctionPointer function); 

    private:

    MultiVButton powerBtn;


};


#endif