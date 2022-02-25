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

    display->setTextWrap(false);
    display->fillScreen(BACKGROUND);
    // Render Battery Label
    display->setFont(&FreeSans9pt7b);
    display->setTextColor(RGB(0, 0, 50));
    display->setCursor(batteryProperties.x + 5, batteryProperties.y + 16);
    display->print("Bat%");
    display->setTextColor(RGB(0, 0, 0));
    display->setCursor(batteryProperties.x, batteryProperties.y + 32);
    display->print("CT:");
    display->setCursor(batteryProperties.x, batteryProperties.y + 48);
    display->print("FC:");
}

void RemoteUI::setBatteryLocation(uint16_t x, uint16_t y) {
    batteryProperties.x = x;
    batteryProperties.y = y;
}

void RemoteUI::setRadioStateLocation(uint16_t x, uint16_t y) {
    receiverProperties.x = x;
    receiverProperties.y = y;
}

void RemoteUI::renderTxBattery(uint8_t txLevel) {
    display->fillRect(batteryProperties.x + 26, batteryProperties.y + 33, 22, -16, txLevel < 20 ? LOWBAT_BACKGROUND : BACKGROUND);
    display->setTextColor(RGB(0, 0, 0));
    display->setCursor(batteryProperties.x + 26, batteryProperties.y + 32);
    if (txLevel == 255) {
        display->print(" --");
    } else {
        if (txLevel > 99) {
            txLevel = 99;
        }
        display->setFont(&FreeSansBold9pt7b);
        display->print(txLevel);
    }
}

void RemoteUI::renderRxBattery(uint8_t rxLevel) {
    display->fillRect(batteryProperties.x + 26, batteryProperties.y + 49, 22, -16, rxLevel < 20 ? LOWBAT_BACKGROUND : BACKGROUND);
    display->setTextColor(RGB(0, 0, 0));
    display->setCursor(batteryProperties.x + 26, batteryProperties.y + 48);
    if (rxLevel == 255) {
        display->print(" --");
    } else {
        if (rxLevel > 99) {
            rxLevel = 99;
        }
        display->setFont(&FreeSansBold9pt7b);
        display->print(rxLevel);
    }
}

void RemoteUI::renderRadioState(uint8_t state, int8_t rxSNR, int8_t txSNR) {
    display->setTextColor(RGB(0, 0, 0));
    display->setCursor(receiverProperties.x, receiverProperties.y + 14);
    display->fillRect(receiverProperties.x, receiverProperties.y + 18, 150, -18, BACKGROUND);
    switch (state) {
    case 0:
        display->setFont(&FreeSans9pt7b);
        display->print("Searching");
        for (int i = 0; i < rxSNR; i++) {
            display->print('.');
        }
        break;
    case 1:
        display->setFont(&FreeSans9pt7b);
        display->print("Syncing");
        break;
    case 2:

        if (rxSNR == 0) {
            display->setTextColor(RGB(25, 0, 0));
            display->setFont(&FreeSansBold9pt7b);
            display->print("Waiting on HB");
            for (int i = 0; i < txSNR; i++) {
                display->print('.');
            }
        } else {
            display->setFont(&FreeSans9pt7b);
            display->print("CT:");
            display->setFont(&FreeSansBold9pt7b);
            display->print(rxSNR);
            display->setFont(&FreeSans9pt7b);
            display->print("dB  FC:");
            display->setFont(&FreeSansBold9pt7b);
            display->print(txSNR);
            display->setFont(&FreeSans9pt7b);
            display->print("dB");
        }
        break;
    }
}