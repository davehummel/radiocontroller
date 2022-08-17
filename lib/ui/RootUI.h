#ifndef root_ui_H__
#define root_ui_H__
#include "RemoteDisplay.h"
#include "VMExecutor.h"

class Screen {
  public:
    virtual String &getTitle() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
};

class RootUI : RunnableTask {
  private:
    RemoteUI &display;

    ScheduledLink *link = NULL;

    uint8_t batteryPercent = 0;
    int8_t ctrlRX = 0, clientRX = 0;

    Screen *currentScreen = NULL;

    bool underlayRendered = false;
    bool titleRendered = false;
    bool batteryRendered = false;
    bool radioRendered = false;

    uint8_t redrawCounter = 0;

    void renderUnderlay();
    void renderTitle();
    void renderTime();
    void renderRadio();
    void renderBattery();

  public:
    RootUI(RemoteUI &display) : display(display){
        display.getDisplay()->setFontMode(1); // Transparent background font
    };

    void start() {
        link = EXECUTOR.schedule(this, EXECUTOR.getTimingPair(500, FrequencyUnitEnum::milli));
        renderUnderlay();
        underlayRendered = true;
    }
    void stop() {
        if (link != NULL) {
            link->cancel();
            link = NULL;
        }
    }

    void run(TIME_INT_t time);

    void setScreen(Screen *newScreen) {
        if (currentScreen != NULL) {
            currentScreen->stop();
        }
        if (newScreen != NULL) {
            newScreen->start();
            currentScreen = newScreen;
            titleRendered = false;
        }
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

extern RootUI ROOT_UI;

#endif
