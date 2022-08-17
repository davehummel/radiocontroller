#ifndef remote__input__controls_H__
#define remote__input__controls_H__
#include "VMExecutor.h"
#include "VMTime.h"
#include <Arduino.h>
#include <InputControls.h>

class RemoteControlInputSet : public InputSet, RunnableTask {
  private:
    uint32_t intervalMicros;

  public:
    enum ARROW_DIR { NONE, UP, LEFT, DOWN, RIGHT };

    RemoteControlInputSet();

    void start(TIME_INT_t intervalMicros);

    void run(TIME_INT_t time);

    uint32_t getIntervalMicros() { return intervalMicros; }

    JoyInput joy1H;
    JoyInput joy1V;
    JoyInput joy2H;
    JoyInput joy2V;

    WheelInput wheel;
    MultiVButton arrows;
    ButtonInput wheelBtn;
    ButtonInput button1;
    ButtonInput button2;
    ButtonInput button3;
    ButtonInput button4;
    ButtonInput button5;
};

extern RemoteControlInputSet CONTROLS;

#endif