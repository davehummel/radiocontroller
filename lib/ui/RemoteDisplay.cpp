#include <RemoteDisplay.h>

#include <FDOS_LOG.h>

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

    
void RemoteUI:: setSPIBusy(){
    spiLock = true;
}
    
void RemoteUI:: setSPIFree(){
    spiLock = false;
    if (redrawMissed){
        requestDraw(true);
    }

}
