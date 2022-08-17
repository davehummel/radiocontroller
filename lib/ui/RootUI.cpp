#include "RootUI.h"
#include "PowerControl.h"

RootUI ROOT_UI(UI);

const String SCREEN_TITLES[] = {"Status", "Settings", "PC Control", "Flight Control", "Select"};

void RootUI::run(TIME_INT_t time) {
    redrawCounter++;
    if (redrawCounter == 100) {
        redrawCounter = 0;
    }
    if (!titleRendered) {
        renderTitle();
        titleRendered = true;
    }
    if (redrawCounter % 2) {
        renderTime();
    }
    if (redrawCounter % 5) {
        if (!radioRendered) {
            renderRadio();
            radioRendered = true;
        }
        if (redrawCounter % 10) {
            setBattery(POWER.getBatteryPercent());
            if (!batteryRendered) {
                renderBattery();
                batteryRendered = true;
            }
        }
    }
}

void RootUI::renderTime() {
    TIME_INT_t now = microsSinceEpoch();
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawBox(185, 0, 100, 24);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->setFont(u8g2_font_10x20_tr);
    display.getDisplay()->setCursor(195, 5);
    display.getDisplay()->printf("%02i:%02i:%02i", getLocalHour(now), getLocalMinute(now), getLocalSecond(now));
    display.requestDraw();
}

void RootUI::renderUnderlay() {
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->drawBox(0, 0, 400, 240);
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawFrame(0, 0, 400, 240);
    display.getDisplay()->drawBox(0, 0, 400, 25);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->drawVLine(184, 0, 21);
    display.getDisplay()->drawVLine(288, 0, 23);
    display.requestDraw();
}

void RootUI::renderTitle() {
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawBox(4, 0, 180, 23);
    if (currentScreen != NULL) {
        display.getDisplay()->setDrawColor(1);
        display.getDisplay()->setFont(u8g2_font_t0_22b_tr);
        display.getDisplay()->drawStr(5, 5, currentScreen->getTitle().c_str());
    }
    display.requestDraw();
}

void RootUI::renderBattery() {
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawBox(375, 1, 14, 24);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->setFont(u8g2_font_battery19_tn);
    display.getDisplay()->setCursor(376, 2);
    display.getDisplay()->print((char)('0' + (batteryPercent + 19) / 20));
    display.requestDraw();
}

void RootUI::renderRadio() {
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawBox(292, 1, 86, 24);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->setFont(u8g2_font_10x20_tr);
    display.getDisplay()->drawStr(293, 6, "T");
    display.getDisplay()->drawStr(331, 6, "R");
    display.getDisplay()->setFont(u8g2_font_t0_14b_te);
    if (ctrlRX != 0) {
        display.getDisplay()->setCursor(304, 8);
        display.getDisplay()->print(ctrlRX);
    } else {
        display.getDisplay()->drawStr(304, 8, "d/c");
    }
    if (clientRX != 0) {
        display.getDisplay()->setCursor(343, 8);
        display.getDisplay()->print(ctrlRX);
    } else {
        display.getDisplay()->drawStr(343, 8, "d/c");
    }
    display.requestDraw();
}