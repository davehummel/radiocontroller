#include "NavScreen.h"

NavScreen NAV_SCREEN;

void NavScreen::start() {
    if (link == NULL) {
        link = EXECUTOR.schedule(this, EXECUTOR.getTimingPair(5, FrequencyUnitEnum::milli));

        drawGuide();
        CONTROLS.button1.setLEDValue(255);
        CONTROLS.button1.subscribe(navActionButtonListener);
        CONTROLS.button2.setLEDValue(255);
        CONTROLS.button2.subscribe(navActionButtonListener);
        CONTROLS.button3.setLEDValue(255);
        CONTROLS.button3.subscribe(navActionButtonListener);
        CONTROLS.button4.setLEDValue(255);
        CONTROLS.button4.subscribe(navActionButtonListener);
        CONTROLS.button5.setLEDValue(255);
        CONTROLS.button5.subscribe(navActionButtonListener);
    }
}

void NavScreen::stop() {
    if (link != NULL) {
        link->cancel();
        link = NULL;

        CONTROLS.button1.setLEDValue(0);
        CONTROLS.button2.setLEDValue(0);
        CONTROLS.button3.setLEDValue(0);
        CONTROLS.button4.setLEDValue(0);
        CONTROLS.button5.setLEDValue(0);

        CONTROLS.button1.unsubscribe(navActionButtonListener);
        CONTROLS.button2.unsubscribe(navActionButtonListener);
        CONTROLS.button3.unsubscribe(navActionButtonListener);
        CONTROLS.button4.unsubscribe(navActionButtonListener);
        CONTROLS.button5.unsubscribe(navActionButtonListener);
    }
}

void NavScreen::run(TIME_INT_t time) {
    CONTROLS.button1.setLEDValue(abs((time / 5000 + 0 * 64) % 512) - 256);
    CONTROLS.button2.setLEDValue(abs((time / 5000 + 1 * 64) % 512) - 256);
    CONTROLS.button3.setLEDValue(abs((time / 5000 + 2 * 64) % 512) - 256);
    CONTROLS.button4.setLEDValue(abs((time / 5000 + 3 * 64) % 512) - 256);
    CONTROLS.button5.setLEDValue(abs((time / 5000 + 4 * 64) % 512) - 256);
}

void NavScreen::drawGuide() {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 215);
    drawNavMenu();
    UI.getDisplay()->setFontDirection(1);
    UI.getDisplay()->setFont(u8g2_font_10x20_tr);

    UI.getDisplay()->drawStr(48, 190 - UI.getDisplay()->getStrWidth("Flight Control"), "Flight Control");
    UI.getDisplay()->drawStr(98, 180 - UI.getDisplay()->getStrWidth("USB Joystick"), "USB Joystick");
    UI.getDisplay()->drawStr(148, 190 - UI.getDisplay()->getStrWidth("Settings"), "Settings");
    UI.getDisplay()->drawStr(298, 180 - UI.getDisplay()->getStrWidth("Status"), "Status");
    UI.getDisplay()->drawStr(348, 190 - UI.getDisplay()->getStrWidth("Display Light"), "Display Light");
    UI.getDisplay()->setFontDirection(0);
    UI.requestDraw();
}

