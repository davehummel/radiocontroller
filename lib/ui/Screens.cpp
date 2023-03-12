#include "Screens.h"
#include "PowerControl.h"
#include "RadioTask.h"
#include "SettingScreen.h"
#include "FlightScreen.h"
#include "NavScreen.h"
#include "PCScreen.h"
#include "StatusScreen.h"


String EMPTY_TITLE = "";

void drawNavMenu(String b5 , String b4 , String b3 , String bCirc , String b2 , String b1 , bool b5Toggle ,
                 bool b4Toggle , bool b3Toggle , bool b2Toggle , bool b1Toggle ) {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(24, 190, 352, 50);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setFont(u8g2_font_t0_14b_te);

    UI.getDisplay()->drawCircle(40, 215, 10);
    if (b5Toggle)
        UI.getDisplay()->drawDisc(40, 215, 7);
    if (b5 != EMPTY_TITLE) {
        uint16_t w = UI.getDisplay()->getStrWidth(b5.c_str());
        UI.getDisplay()->drawStr(40 - w / 2, 190, b5.c_str());
    }

    UI.getDisplay()->drawCircle(90, 202, 10);
    if (b4Toggle)
        UI.getDisplay()->drawDisc(90, 202, 7);
    if (b4 != EMPTY_TITLE) {
        uint16_t w = UI.getDisplay()->getStrWidth(b4.c_str());
        UI.getDisplay()->drawStr(90 - w / 2, 217, b4.c_str());
    }

    UI.getDisplay()->drawCircle(140, 215, 10);
    if (b3Toggle)
        UI.getDisplay()->drawDisc(140, 215, 7);
    if (b3 != EMPTY_TITLE) {
        uint16_t w = UI.getDisplay()->getStrWidth(b3.c_str());
        UI.getDisplay()->drawStr(140 - w / 2, 190, b3.c_str());
    }

    UI.getDisplay()->drawCircle(290, 202, 10);
    if (b2Toggle)
        UI.getDisplay()->drawDisc(290, 202, 7);
    if (b2 != EMPTY_TITLE) {
        uint16_t w = UI.getDisplay()->getStrWidth(b2.c_str());
        UI.getDisplay()->drawStr(290 - w / 2, 217, b2.c_str());
    }

    UI.getDisplay()->drawCircle(340, 215, 10);
    if (b1Toggle)
        UI.getDisplay()->drawDisc(340, 215, 7);
    if (b1 != EMPTY_TITLE) {
        uint16_t w = UI.getDisplay()->getStrWidth(b1.c_str());
        UI.getDisplay()->drawStr(340 - w / 2, 190, b1.c_str());
    }

    UI.getDisplay()->drawDisc(200, 212, 6);
    UI.getDisplay()->drawCircle(200, 212, 15);
    UI.getDisplay()->drawCircle(200, 212, 20);
    if (bCirc != EMPTY_TITLE) {
        UI.getDisplay()->drawStr(226, 194, bCirc.c_str());
    }
}
void drawXYInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t xVal, uint16_t yVal, uint16_t valMax, bool printVals) {

    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, size, size);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + (size * xVal) / valMax - 1, topY + size - (size * yVal) / valMax - 1, 3, 3);
    if (printVals) {
        const uint8_t NUM_SIZE = 40;
        UI.getDisplay()->setDrawColor(2);
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->setCursor(topX + 16, topY + (size - NUM_SIZE) / 2);
        UI.getDisplay()->printf("%04d", yVal);
        UI.getDisplay()->setFontDirection(0);
        UI.getDisplay()->setCursor(topX + (size - NUM_SIZE) / 2, topY + size - 16);
        UI.getDisplay()->printf("%04d", xVal);
    }
}

const uint8_t NUM_SIZE = 40;
void drawHInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, size, width);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + (val * size / valMax), topY + 1, size - val * size / valMax, width - 2);
    if (printVals) {
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);

        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->setCursor(topX + (size - NUM_SIZE) / 2, topY + width + 2);
        UI.getDisplay()->printf("%04d", val);
    }
}

void drawVInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals, bool printLeftSide ) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, width, size);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + 1, topY, width - 2, size - (val * size) / valMax);
    if (printVals) {
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->setCursor(printLeftSide ? topX - 2 : topX + width + 12, topY + (size - NUM_SIZE) / 2);
        UI.getDisplay()->printf("%04d", val);
        UI.getDisplay()->setFontDirection(0);
    }
}




void navEnableButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        ROOT_UI.setScreen((Screen *)&NAV_SCREEN);
    }
}

void navActionButtonListener() {
    if (CONTROLS.button5.isPressed()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_SCREEN);
        return;
    }

    if (CONTROLS.button4.isPressed()) {
        ROOT_UI.setScreen((Screen *)&PC_SCREEN);
        return;
    }
    if (CONTROLS.button3.isPressed()) {
        ROOT_UI.setScreen((Screen *)&SETTINGS_SCREEN);
        return;
    }
    if (CONTROLS.button2.isPressed()) {
        ROOT_UI.setScreen((Screen *)&STATUS_SCREEN);
        return;
    }
    if (CONTROLS.button1.isPressed()) {
        UI.illuminate(!UI.isIlluminated());
        return;
    }
}

