#include "RootUI.h"
#include "PowerControl.h"

RootUI ROOT_UI(UI);

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
    display.getDisplay()->drawBox(235, 0, 100, 24);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->setFont(u8g2_font_10x20_tr);
    display.getDisplay()->setCursor(245, 5);
    display.getDisplay()->printf("%02i:%02i:%02i", getLocalHour(now), getLocalMinute(now), getLocalSecond(now));
    display.requestDraw();
}

void RootUI::renderUnderlay() {
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->drawBox(0, 0, 400, 240);
    display.getDisplay()->setDrawColor(0);
    display.getDisplay()->drawFrame(0, 0, 400, 240);
    display.getDisplay()->drawBox(0, 0, 400, 25);
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
    display.getDisplay()->drawBox(372, 1, 14, 24);
    display.getDisplay()->setDrawColor(1);
    display.getDisplay()->setFont(u8g2_font_battery19_tn);
    display.getDisplay()->setCursor(373, 2);
    display.getDisplay()->print((char)('0' + (batteryPercent + 19) / 20));
    display.requestDraw();
}

