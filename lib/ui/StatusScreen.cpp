#include "StatusScreen.h"
#include "PowerControl.h"

void arrowListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.RIGHT)
        STATUS_SCREEN.pageRight();
    else if (CONTROLS.arrows.getState() == CONTROLS.LEFT)
        STATUS_SCREEN.pageLeft();
}


void StatusScreen::start() {
    if (link == NULL) {
        link = EXECUTOR.schedule(this, EXECUTOR.getTimingPair(500, FrequencyUnitEnum::milli));
        page = 1;
        CONTROLS.arrows.subscribe(arrowListener);
        CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
        CONTROLS.button1.subscribe(navActionButtonListener);
        CONTROLS.button1.setLEDValue(200);
        CONTROLS.button2.setLEDValue(200);
        CONTROLS.button2.subscribe(navActionButtonListener);
        CONTROLS.button3.setLEDValue(200);
        CONTROLS.button3.subscribe(navActionButtonListener);
        CONTROLS.button4.setLEDValue(200);
        CONTROLS.button4.subscribe(navActionButtonListener);
        CONTROLS.button5.setLEDValue(200);
        CONTROLS.button5.subscribe(navActionButtonListener);
        updateRootUI();
    }
}

void StatusScreen::stop() {
    if (link != NULL) {
        link->cancel();
        link = NULL;
        CONTROLS.arrows.unsubscribe(arrowListener);
        CONTROLS.button1.setLEDValue(0);
        CONTROLS.button2.setLEDValue(0);
        CONTROLS.button3.setLEDValue(0);
        CONTROLS.button4.setLEDValue(0);
        CONTROLS.button5.setLEDValue(0);
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
        CONTROLS.button1.unsubscribe(navActionButtonListener);
        CONTROLS.button2.unsubscribe(navActionButtonListener);
        CONTROLS.button3.unsubscribe(navActionButtonListener);
        CONTROLS.button4.unsubscribe(navActionButtonListener);
        CONTROLS.button5.unsubscribe(navActionButtonListener);
    }
}

void StatusScreen::run(TIME_INT_t time) { renderPage(); }

void StatusScreen::drawMenu(const char *text, bool isLeft) {

    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setFont(u8g2_font_10x20_tr);

    if (isLeft) {
        UI.getDisplay()->drawRBox(-6, 26, 32, 180, 8);
        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->drawStr(22, 50, text);
        UI.getDisplay()->setFontDirection(0);
        UI.getDisplay()->drawTriangle(23, 130, 2, 138, 23, 146);
    } else {
        UI.getDisplay()->drawRBox(372, 26, 32, 180, 8);
        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->drawStr(392, 50, text);
        UI.getDisplay()->setFontDirection(0);
        UI.getDisplay()->drawTriangle(377, 130, 398, 138, 377, 146);
    }
}

void StatusScreen::updateRootUI() {
    switch (page) {
    case 0:

        drawMenu("Input", true);
        drawMenu("Summary", false);
        break;
    case 1:

        drawMenu("Config", true);
        drawMenu("Input", false);
        break;
    case 2:

        drawMenu("Summary", true);
        drawMenu("Config", false);
        break;
    }

    drawNavMenu("Fly", "USB Joy", "Settings", "Exit",EMPTY_TITLE,"Light",true,true,true,false,true);

    UI.requestDraw();
}

void StatusScreen::renderPage() {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(24, 25, 352, 165);
    UI.getDisplay()->setDrawColor(0);
    switch (page) {
    case 0: // Summary
        break;
    case 1:
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->setCursor(35, 30);
        UI.getDisplay()->print("Date ");
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        {
            TIME_INT_t now = microsSinceEpoch();
            UI.getDisplay()->printf("%02i/%02i/%04i %02i:%02i:%02i", getLocalMonth(now), getLocalDay(now), getLocalYear(now), getLocalHour(now),
                                    getLocalMinute(now), getLocalSecond(now));

            UI.getDisplay()->setCursor(35, 50);
            UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
            UI.getDisplay()->print(" on for ");
            UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
            int32_t onTimeSec = micros64() / 1000000ll;

            UI.getDisplay()->printf("%d h %d m %d s", onTimeSec / 3600, (onTimeSec / 60) % 60, onTimeSec % 60);
            UI.getDisplay()->setCursor(35, 70);
            UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
            UI.getDisplay()->print(" total ");
            UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
            int32_t totalOnTimeSec = SETTINGS.getRuntimeSeconds();
            UI.getDisplay()->printf("%d h %d m %d s", totalOnTimeSec / 3600, (totalOnTimeSec / 60) % 60, totalOnTimeSec % 60);
        }

        UI.getDisplay()->setCursor(35, 90);
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->print("Power ");
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        UI.getDisplay()->printf("%i%% %0.2fv", POWER.getBatteryPercent(), POWER.getBatteryVoltage());

        UI.getDisplay()->setCursor(35, 110);
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->print("Radio ");
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        UI.getDisplay()->printf("Freq %.1f LkBW ", field_radio_Freq.getValue().f);
        switch (field_radio_Linkbw.getValue().u8) {
        case 0:
            UI.getDisplay()->print("100");
            break;
        case 1:
            UI.getDisplay()->print("250");
            break;
        case 2:
            UI.getDisplay()->print("500");
            break;
        }
        UI.getDisplay()->setCursor(35, 128);
        UI.getDisplay()->printf("   Sprd %i Pwr %i", field_radio_SpreadingFactor.getValue().u8, field_radio_Power.getValue().u8);
#ifdef RADIO_AGRESSIVE_RECEIVE_SWITCH
        UI.getDisplay()->printf(" Fast Sw");
#endif
        break;
    case 2:
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->setCursor(35, 30);
        UI.getDisplay()->print("Joy 1 --------------> ");
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);

        UI.getDisplay()->setCursor(35, 50);
        UI.getDisplay()->printf("H %i(%i) V %i(%i)", CONTROLS.joy1H.getSignedValue(), CONTROLS.joy1H.getRawValue(), CONTROLS.joy1V.getUnsignedValue(),
                                CONTROLS.joy1V.getRawValue());
        UI.getDisplay()->setCursor(35, 80);
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->print("    <-------------- Joy 2 ");
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);

        UI.getDisplay()->setCursor(35, 100);
        UI.getDisplay()->printf("H %i(%i) V %i(%i)", CONTROLS.joy2H.getSignedValue(), CONTROLS.joy2H.getRawValue(), CONTROLS.joy2V.getSignedValue(),
                                CONTROLS.joy2V.getRawValue());

        break;
    }
    UI.requestDraw();
}

void StatusScreen::pageLeft() {
    if (page == 0)
        page = 2;
    else
        page--;
    updateRootUI();
    renderPage();
}

void StatusScreen::pageRight() {
    if (page == 2)
        page = 0;
    else
        page++;
    updateRootUI();
    renderPage();
}


StatusScreen STATUS_SCREEN;
