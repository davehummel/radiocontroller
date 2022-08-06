#ifndef root_ui_H__
#define root_ui_H__
#include "RemoteDisplay.h"
#include "VMExecutor.h"

enum UI_SCREENS { STARTUP, SETUP, PC, FLIGHT };
String SCREEN_TITLES[] = {"Startup","Settings","PC Control","Flight Control"};

class RootUI : RunnableTask {
  private:
    RemoteUI *display;

    uint8_t batteryPercent = 100;
    uint8_t ctrlRX = 0, clientRX = 0;

    UI_SCREENS screen;
    bool underlayRendered;
    bool titleRendered;
    bool batteryRendered;
    bool radioRendered;

    uint8_t redrawCounter = 0;

    void renderUnderlay();
    void renderTitle();
    void renderRadio();
    void renderBattery();


  public:
    RootUI(RemoteUI *display) : display(display){};

    void run(TIME_INT_t time);

    void setScreen(UI_SCREENS newScreen) {
        if (screen == newScreen)
            return;
        screen = newScreen;
        titleRendered = false;
    }

    void setBattery(uint8_t percent) {
        if (batteryPercent == percent)
            return;
        batteryPercent = percent;
        batteryRendered = false;
    }

    void setRadioSignal(uint8_t newCtrlRX, uint8_t newClientRX) {
        if (newClientRX == clientRX && newCtrlRX == ctrlRX)
            return;
        ctrlRX = newCtrlRX;
        clientRX = newClientRX;
        radioRendered = false;
    }
};

#endif
