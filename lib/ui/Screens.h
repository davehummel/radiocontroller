#ifndef screens_H__
#define screens_H__
#include "RemoteControlInputs.h"
#include "RemoteDisplay.h"
#include "RootUI.h"



extern String EMPTY_TITLE;

void navEnableButtonListener();
void navActionButtonListener();

void drawNavMenu(String b5 = EMPTY_TITLE, String b4 = EMPTY_TITLE, String b3 = EMPTY_TITLE, String bCirc = EMPTY_TITLE, String b2 = EMPTY_TITLE, String b1 = EMPTY_TITLE, bool b5Toggle = false,
                 bool b4Toggle = false, bool b3Toggle = false, bool b2Toggle = false, bool b1Toggle = false,bool minimal = false) ;
void drawXYInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t xVal, uint16_t yVal, uint16_t valMax, bool printVals);
void drawHInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals);
void drawVInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals, bool printLeftSide = true);




#endif
