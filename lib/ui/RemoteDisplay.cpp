#include <RemoteDisplay.h>

#include <FDOS_LOG.h>
#include <SPILock.h>

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
        uint16_t spiLockID = 0;
        if (!SPILOCK.tryLock(spiLockID)) {
            redrawMissed = true;
            return;
        }
        display->sendBuffer();
        SPILOCK.releaseLock(spiLockID);
        redrawNeeded = false;
    } else {
        redrawNeeded = true;
    }
}


void RemoteUI::illuminate(bool isOn){
    digitalWrite(DISPLAY_LIGHT_PIN,isOn);
    illuminated = isOn;
}

bool RemoteUI::isIlluminated(){
    return illuminated;
}