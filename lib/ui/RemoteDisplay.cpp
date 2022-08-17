#include <RemoteDisplay.h>

#include <FDOS_LOG.h>

RemoteUI UI = RemoteUI();

void RemoteUI::start() {
    display->setFontPosTop();
    EXECUTOR.schedule(this,EXECUTOR.getTimingPair(50,FrequencyUnitEnum::milli));
}

void RemoteUI::run(TIME_INT_t time) {
    if (redrawNeeded) {
        requestDraw(true);
    }
}

void RemoteUI::requestDraw(bool immediate) {

    if (immediate) {
        if (spiLock) {
            redrawMissed = true;
            return;
        }
        display->sendBuffer();
        redrawNeeded = false;
    } else {
        redrawNeeded = true;
    }
}

void RemoteUI::setSPIBusy() { spiLock = true; }

void RemoteUI::setSPIFree() {
    spiLock = false;
    if (redrawMissed) {
        requestDraw(true);
    }
}

void RemoteUI::illuminate(bool isOn){
    digitalWrite(DISPLAY_LIGHT_PIN,isOn);
    illuminated = isOn;
}

bool RemoteUI::isIlluminated(){
    return illuminated;
}