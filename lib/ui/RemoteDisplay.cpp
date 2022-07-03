#include <RemoteDisplay.h>

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <FDOS_LOG.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define RGB(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

const uint16_t BACKGROUND = RGB(255, 255, 255);
const uint16_t LOWBAT_BACKGROUND = RGB(255, 200, 200);

RemoteUI::RemoteUI() {}

void RemoteUI::renderBackground() {

    //display->setTextWrap(false);
    //display->fillScreen(BACKGROUND);
    // Render Battery Label
    //display->setFont(&FreeSans9pt7b);
    //display->setTextColor(RGB(0, 0, 50));
    //display->setCursor(batteryProperties.x + 5, batteryProperties.y + 16);
    //display->print("Bat%");
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(batteryProperties.x, batteryProperties.y + 32);
    //display->print("CT:");
    //display->setCursor(batteryProperties.x, batteryProperties.y + 48);
    //display->print("FC:");
}

void RemoteUI::setBatteryLocation(uint16_t x, uint16_t y) {
    batteryProperties.x = x;
    batteryProperties.y = y;
}

void RemoteUI::setInputLocation(uint16_t x, uint16_t y) {
    inputProperties.x = x;
    inputProperties.y = y;
}

void RemoteUI::setRadioStateLocation(uint16_t x, uint16_t y) {
    receiverProperties.x = x;
    receiverProperties.y = y;
}

void RemoteUI::setOrientationLocation(uint16_t x, uint16_t y) {
    orientationProperties.x = x;
    orientationProperties.y = y;
}

void RemoteUI::renderCTBattery(uint8_t ctLevel) {
    //display->fillRect(batteryProperties.x + 26, batteryProperties.y + 33, 22, -16, ctLevel < 20 ? LOWBAT_BACKGROUND : BACKGROUND);
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(batteryProperties.x + 26, batteryProperties.y + 32);
    if (ctLevel == 255) {
        //display->print(" --");
    } else {
        if (ctLevel > 99) {
            ctLevel = 99;
        }
        //display->setFont(&FreeSansBold9pt7b);
        //display->print(ctLevel);
    }
}

void RemoteUI::renderFCBattery(uint8_t fcLevel) {
    //display->fillRect(batteryProperties.x + 26, batteryProperties.y + 49, 22, -16, fcLevel < 20 ? LOWBAT_BACKGROUND : BACKGROUND);
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(batteryProperties.x + 26, batteryProperties.y + 48);
    if (fcLevel == 255) {
        //display->print(" --");
    } else {
        if (fcLevel > 99) {
            fcLevel = 99;
        }
        //display->setFont(&FreeSansBold9pt7b);
        //display->print(fcLevel);
    }
}

void RemoteUI::renderRadioState(uint8_t state, int8_t rxSNR, int8_t txSNR) {
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(receiverProperties.x, receiverProperties.y + 14);
    //display->fillRect(receiverProperties.x, receiverProperties.y + 18, 160, -18, BACKGROUND);
    switch (state) {
    case 0:
        //display->setFont(&FreeSans9pt7b);
        //display->print("Searching");
        for (int i = 0; i < rxSNR; i++) {
            //display->print('.');
        }
        break;
    case 1:
        //display->setFont(&FreeSans9pt7b);
        //display->print("Syncing");
        break;
    case 2:

        if (rxSNR == 0) {
            //display->setTextColor(RGB(25, 0, 0));
            //display->setFont(&FreeSansBold9pt7b);
            //display->print("Waiting on HB");
            for (int i = 0; i < txSNR; i++) {
                //display->print('.');
            }
        } else {
            //display->setFont(&FreeSans9pt7b);
            //display->print("CT:");
            //display->setFont(&FreeSansBold9pt7b);
            //display->print(txSNR);
            //display->setFont(&FreeSans9pt7b);
            //display->print("dB  FC:");
            //display->setFont(&FreeSansBold9pt7b);
            //display->print(rxSNR);
            //display->setFont(&FreeSans9pt7b);
            //display->print("dB");
        }
        break;
    }
}

void RemoteUI::renderInput(int8_t joyH, int8_t joyV, int8_t slideH, uint8_t slideV) {
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(inputProperties.x, inputProperties.y + 14);
    //display->fillRect(inputProperties.x, inputProperties.y + 18, 168, -18, RGB(225, 220, 225));
    //display->setFont(&FreeSans9pt7b);
    //display->print("jh:");
    //display->setFont(&FreeSansBold9pt7b);
    //display->print(joyH);
    //display->setFont(&FreeSans9pt7b);
    //display->print(" jv:");
    //display->setFont(&FreeSansBold9pt7b);
    //display->print(joyV);
    //display->setFont(&FreeSans9pt7b);
    //display->print(" sh:");
    //display->setFont(&FreeSansBold9pt7b);
    //display->print(slideH);
    //display->setFont(&FreeSans9pt7b);
    //display->print(" sv:");
    //display->setFont(&FreeSansBold9pt7b);
    //display->print(slideV);
}

void RemoteUI::renderMotors(uint8_t ul, uint8_t ur, uint8_t bl, uint8_t br) {
    //display->setTextColor(RGB(0, 0, 0));
    //display->setCursor(motorProperties.x, motorProperties.y + 14);
    //display->fillRect(motorProperties.x, motorProperties.y + 32, 72, -32,  RGB(255, 200, 255));

    //display->setFont(&FreeSans9pt7b);
    //display->print(ul);
    //display->setCursor(motorProperties.x, motorProperties.y + 30);
    //display->print(bl);
    //display->setCursor(motorProperties.x + 36, motorProperties.y + 14);
    //display->print(ur);
    //display->setCursor(motorProperties.x + 36, motorProperties.y + 30);
    //display->print(br);
}

void RemoteUI::renderOrientation(float targetY, float actualY, float targetP, float actualP, float targetR, float actualR) {
    //display->setTextColor(RGB(0, 0, 0));
    //display->fillRect(orientationProperties.x, orientationProperties.y + 48, 142, -48, RGB(180, 255, 255));

        //display->setFont(&FreeSansBold9pt7b);
    //display->setCursor(orientationProperties.x, orientationProperties.y + 30);
    //display->print("T");
    //display->setCursor(orientationProperties.x, orientationProperties.y + 46);
    //display->print("C");

    //display->setCursor(orientationProperties.x + 18, orientationProperties.y + 14);
    //display->print("Yaw");
    //display->setCursor(orientationProperties.x + 58, orientationProperties.y + 14);
    //display->print("Pch");
    //display->setCursor(orientationProperties.x + 98, orientationProperties.y + 14);
    //display->print("Rll");
    //display->setFont(&FreeSans9pt7b);
    //display->setCursor(orientationProperties.x + 15, orientationProperties.y + 30);
    //display->print(targetY, 0);
    //display->setCursor(orientationProperties.x + 60, orientationProperties.y + 30);
    //display->print(targetP, 0);
    //display->setCursor(orientationProperties.x + 102, orientationProperties.y + 30);
    //display->print(targetR, 0);

    //display->setCursor(orientationProperties.x + 15, orientationProperties.y + 46);
    //display->print(actualY, 0);
    //display->setCursor(orientationProperties.x + 60, orientationProperties.y + 46);
    //display->print(actualP, 0);
    //display->setCursor(orientationProperties.x + 102, orientationProperties.y + 46);
    //display->print(actualR, 0);

    //display->setCursor(orientationProperties.x, orientationProperties.y + 30);
}