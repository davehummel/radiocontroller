#include "RootUI.h"

void RootUI::run(TIME_INT_t time){
    redrawCounter++;
    if (redrawCounter == 100){
        redrawCounter = 0;
    }
    if (!underlayRendered){
        renderUnderlay();
        underlayRendered = true;
    }
    if (!titleRendered){
        renderTitle();
        titleRendered = true;
    }
    if (redrawCounter%10){
        if (!radioRendered){
            renderRadio();
            radioRendered = true;
        }
        if (redrawCounter%100){
            if (!batteryRendered){
                renderBattery();
                batteryRendered = true;
            }
        }
    }
    

}