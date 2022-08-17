#include <RemoteControlInputs.h>

RemoteControlInputSet CONTROLS;

RemoteControlInputSet::RemoteControlInputSet()
    : InputSet(12), joy1H(JOY1_H_PIN), joy1V(JOY1_V_PIN), joy2H(JOY2_H_PIN), joy2V(JOY2_V_PIN), wheel(WHEEL_QDA, WHEEL_QDB), arrows(WHEEL_ARROW_PIN, 4, 50),
      wheelBtn(WHEEL_CENTER_BTN_PIN), button1(BTN1_PRESS_PIN, BTN1_LED_PIN), button2(BTN2_PRESS_PIN, BTN2_LED_PIN), button3(BTN3_PRESS_PIN, BTN3_LED_PIN),
      button4(BTN4_PRESS_PIN, BTN4_LED_PIN), button5(BTN5_PRESS_PIN, BTN5_LED_PIN) {
    joy1H.inverted = true;
    joy1V.inverted = true;
    addInput(joy1H, 0);
    addInput(joy1V, 1);
    addInput(joy2H, 2);
    addInput(joy2V, 3);
    addInput(wheel, 4);
    addInput(arrows, 5);
    addInput(wheelBtn, 6);
    addInput(button1, 7);
    addInput(button2, 8);
    addInput(button3, 9);
    addInput(button4, 10);
    addInput(button5, 11);
    arrows.setStateValue(1, 1000);
    arrows.setStateValue(2, 1987);
    arrows.setStateValue(3, 2894);
    arrows.setStateValue(4, 4079);
}

void RemoteControlInputSet::start(TIME_INT_t _intervalMicros) {
    intervalMicros = _intervalMicros;
    EXECUTOR.schedule((RunnableTask *)&CONTROLS, intervalMicros, intervalMicros);
}

void RemoteControlInputSet::run(TIME_INT_t time) { update(); }