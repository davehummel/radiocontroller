#include "PCScreen.h"



void PCScreen::start() {
    if (link == NULL) {
        Joystick.useManualSend(true);
        Joystick.begin();
        CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
        link = EXECUTOR.schedule((RunnableTask *)this, 0, CONTROLS.getIntervalMicros());

        CONTROLS.joy1H.setHalfRange(512);
        CONTROLS.joy1V.setHalfRange(512);
        CONTROLS.joy2H.setHalfRange(512);
        CONTROLS.joy2V.setHalfRange(512);

        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 25, 400, 215);
    }
}
void PCScreen::stop() {
    if (link != NULL) {
        Joystick.end();
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
        link->cancel();
        link = NULL;
    }
}

void PCScreen::run(TIME_INT_t time) {
    updateJoystick();
    runCount++;
    if (runCount == 10) {
        runCount = 0;
        updateScreen();
    }
}

void PCScreen::updateJoystick() {
    CONTROLS.button1.setLEDValue(CONTROLS.button1.isPressed() * 255);
    CONTROLS.button2.setLEDValue(CONTROLS.button2.isPressed() * 255);
    CONTROLS.button3.setLEDValue(CONTROLS.button3.isPressed() * 255);
    CONTROLS.button4.setLEDValue(CONTROLS.button4.isPressed() * 255);
    CONTROLS.button5.setLEDValue(CONTROLS.button5.isPressed() * 255);

    Joystick.button(1, CONTROLS.button2.isPressed());
    Joystick.button(2, CONTROLS.button1.isPressed());
    Joystick.button(3, CONTROLS.button5.isPressed());
    Joystick.button(4, CONTROLS.button4.isPressed());
    Joystick.button(5, CONTROLS.button3.isPressed());
    switch (CONTROLS.arrows.getState()) {
    case 0:
        Joystick.hat(-1);
        break;
    case CONTROLS.LEFT:
        Joystick.hat(270);
        break;
    case CONTROLS.RIGHT:
        Joystick.hat(90);
        break;
    case CONTROLS.UP:
        Joystick.hat(0);
        break;
    case CONTROLS.DOWN:
        Joystick.hat(180);
        break;
    }
    wheelLoc += CONTROLS.wheel.getDelta() * 4;
    if (wheelLoc > 1023)
        wheelLoc = 1023;
    else if (wheelLoc < 0)
        wheelLoc = 0;
    Joystick.sliderRight(wheelLoc);
    Joystick.sliderLeft(CONTROLS.joy1V.getUnsignedValue());
    Joystick.Zrotate(CONTROLS.joy1H.getUnsignedValue());
    Joystick.X(CONTROLS.joy2H.getUnsignedValue());
    Joystick.Y(CONTROLS.joy2V.getUnsignedValue());
    Joystick.send_now();
}

void PCScreen::updateScreen() {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 217);
    drawXYInput(2, 26, 175, CONTROLS.joy2H.getUnsignedValue(), CONTROLS.joy2V.getUnsignedValue(), 1023, true);
    drawHInput(200, 35, 180, 20, CONTROLS.joy1H.getUnsignedValue(), 1023, true);

    drawVInput(210, 70, 120, 20, CONTROLS.joy1V.getUnsignedValue(), 1023, true);
    drawVInput(250, 70, 120, 20, wheelLoc, 1023, true);

    UI.getDisplay()->drawCircle(330, 130, 40);
    switch (CONTROLS.arrows.getState()) {
    case 0:
        break;
    case CONTROLS.LEFT:
        UI.getDisplay()->drawTriangle(292, 130, 302, 125, 302, 135);
        break;
    case CONTROLS.RIGHT:
        UI.getDisplay()->drawTriangle(368, 130, 358, 125, 358, 135);
        break;
    case CONTROLS.UP:
        UI.getDisplay()->drawTriangle(330, 92, 325, 102, 335, 102);
        break;
    case CONTROLS.DOWN:
        UI.getDisplay()->drawTriangle(330, 168, 325, 158, 335, 158);
        break;
    }

    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawDisc(200, 212, 6);
    UI.getDisplay()->drawCircle(200, 212, 15);
    UI.getDisplay()->drawCircle(200, 212, 20);
    UI.getDisplay()->drawHLine(209, 212, 18);
    UI.getDisplay()->setFont(u8g2_font_10x20_mr);
    UI.getDisplay()->drawStr(230, 205, "Exit");
    UI.requestDraw();
}

PCScreen PC_SCREEN;