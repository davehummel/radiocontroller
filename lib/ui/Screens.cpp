#include "Screens.h"
#include "PowerControl.h"
#include "SettingsStore.h"

StatusScreen STATUS_SCREEN;
NavScreen NAV_SCREEN;
SettingsScreen SETTINGS_SCREEN;
FlightScreen FLIGHT_SCREEN;
PCScreen PC_SCREEN;

void drawXYInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t xVal, uint yVal, uint16_t valMax, bool printVals) {

    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, size, size);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + (size * xVal) / valMax - 1, topY + size - (size * yVal) / valMax - 1, 3, 3);
    if (printVals) {
        const uint8_t NUM_SIZE = 40;
        UI.getDisplay()->setDrawColor(2);
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->setCursor(topX + 16, topY + (size - NUM_SIZE) / 2);
        UI.getDisplay()->printf("%04d", yVal);
        UI.getDisplay()->setFontDirection(0);
        UI.getDisplay()->setCursor(topX + (size - NUM_SIZE) / 2, topY + size - 16);
        UI.getDisplay()->printf("%04d", xVal);
    }
}

const uint8_t NUM_SIZE = 40;
void drawHInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, size, width);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + (val * size / valMax), topY + 1, size - val * size / valMax, width - 2);
    if (printVals) {
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);

        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->setCursor(topX + (size - NUM_SIZE) / 2, topY + width + 2);
        UI.getDisplay()->printf("%04d", val);
    }
}

void drawVInput(uint16_t topX, uint16_t topY, uint16_t size, uint16_t width, uint16_t val, uint16_t valMax, bool printVals) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawBox(topX, topY, width, size);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(topX + 1, topY, width - 2, size - (val * size) / valMax);
    if (printVals) {
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);
        UI.getDisplay()->setFontDirection(1);
        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->setCursor(topX - 2, topY + (size - NUM_SIZE) / 2);
        UI.getDisplay()->printf("%04d", val);
        UI.getDisplay()->setFontDirection(0);
    }
}

void arrowListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.RIGHT)
        STATUS_SCREEN.pageRight();
    else if (CONTROLS.arrows.getState() == CONTROLS.LEFT)
        STATUS_SCREEN.pageLeft();
}

void navEnableButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        ROOT_UI.setScreen((Screen *)&NAV_SCREEN);
    }
}

void navActionButtonListener() {
    if (CONTROLS.button5.isPressed()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_SCREEN);
        return;
    }

    if (CONTROLS.button4.isPressed()) {
        ROOT_UI.setScreen((Screen *)&PC_SCREEN);
        return;
    }
    if (CONTROLS.button3.isPressed()) {
        ROOT_UI.setScreen((Screen *)&SETTINGS_SCREEN);
        return;
    }
    if (CONTROLS.button2.isPressed()) {
        ROOT_UI.setScreen((Screen *)&STATUS_SCREEN);
        return;
    }
    if (CONTROLS.button1.isPressed()) {
        UI.illuminate(!UI.isIlluminated());
        return;
    }
}

void NavScreen::start() {
    if (link == NULL) {
        link = EXECUTOR.schedule(this, EXECUTOR.getTimingPair(5, FrequencyUnitEnum::milli));

        drawGuide();
        CONTROLS.button1.setLEDValue(255);
        CONTROLS.button1.subscribe(navActionButtonListener);
        CONTROLS.button2.setLEDValue(255);
        CONTROLS.button2.subscribe(navActionButtonListener);
        CONTROLS.button3.setLEDValue(255);
        CONTROLS.button3.subscribe(navActionButtonListener);
        CONTROLS.button4.setLEDValue(255);
        CONTROLS.button4.subscribe(navActionButtonListener);
        CONTROLS.button5.setLEDValue(255);
        CONTROLS.button5.subscribe(navActionButtonListener);
    }
}

void NavScreen::stop() {
    if (link != NULL) {
        link->cancel();
        link = NULL;

        CONTROLS.button1.setLEDValue(0);
        CONTROLS.button2.setLEDValue(0);
        CONTROLS.button3.setLEDValue(0);
        CONTROLS.button4.setLEDValue(0);
        CONTROLS.button5.setLEDValue(0);

        CONTROLS.button1.unsubscribe(navActionButtonListener);
        CONTROLS.button2.unsubscribe(navActionButtonListener);
        CONTROLS.button3.unsubscribe(navActionButtonListener);
        CONTROLS.button4.unsubscribe(navActionButtonListener);
        CONTROLS.button5.unsubscribe(navActionButtonListener);
    }
}

void NavScreen::run(TIME_INT_t time) {
    CONTROLS.button1.setLEDValue(abs((time / 5000 + 0 * 64) % 512) - 256);
    CONTROLS.button2.setLEDValue(abs((time / 5000 + 1 * 64) % 512) - 256);
    CONTROLS.button3.setLEDValue(abs((time / 5000 + 2 * 64) % 512) - 256);
    CONTROLS.button4.setLEDValue(abs((time / 5000 + 3 * 64) % 512) - 256);
    CONTROLS.button5.setLEDValue(abs((time / 5000 + 4 * 64) % 512) - 256);
}

void NavScreen::drawGuide() {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 217);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawDisc(40, 215, 10);
    UI.getDisplay()->drawDisc(90, 205, 10);
    UI.getDisplay()->drawDisc(140, 215, 10);
    UI.getDisplay()->drawDisc(290, 205, 10);
    UI.getDisplay()->drawDisc(340, 215, 10);
    UI.getDisplay()->drawCircle(200, 212, 20);
    UI.getDisplay()->setFontDirection(1);
    UI.getDisplay()->setFont(u8g2_font_10x20_tr);

    UI.getDisplay()->drawStr(48, 190 - UI.getDisplay()->getStrWidth("Flight Control"), "Flight Control");
    UI.getDisplay()->drawStr(98, 180 - UI.getDisplay()->getStrWidth("PC Control"), "PC Control");
    UI.getDisplay()->drawStr(148, 190 - UI.getDisplay()->getStrWidth("Settings"), "Settings");
    UI.getDisplay()->drawStr(298, 180 - UI.getDisplay()->getStrWidth("Status"), "Status");
    UI.getDisplay()->drawStr(348, 190 - UI.getDisplay()->getStrWidth("Display Light"), "Display Light");
    UI.getDisplay()->setFontDirection(0);
    UI.requestDraw();
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
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(24, 190, 352, 50);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawCircle(40, 215, 10);
    UI.getDisplay()->drawCircle(90, 205, 10);
    UI.getDisplay()->drawCircle(140, 215, 10);
    UI.getDisplay()->drawCircle(290, 205, 10);
    UI.getDisplay()->drawCircle(340, 215, 10);

    UI.getDisplay()->drawDisc(200, 212, 6);
    UI.getDisplay()->drawCircle(200, 212, 15);
    UI.getDisplay()->drawCircle(200, 212, 20);
    UI.getDisplay()->drawHLine(209, 212, 18);
    UI.getDisplay()->setFont(u8g2_font_10x20_tr);
    UI.getDisplay()->drawStr(230, 205, "Nav");
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
        UI.getDisplay()->printf("Freq %.1f LkBW %i", RADIO_CARRIER_FREQ, RADIO_LINK_BANDWIDTH);
        UI.getDisplay()->setCursor(35, 128);
        UI.getDisplay()->printf("   Sprd %i Pwr %i", RADIO_SPREADING_FACTOR, RADIO_POWER);
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

void exitSettingsButtonListener() {
    if (CONTROLS.button5.isPressed()) {
        ROOT_UI.setScreen((Screen *)&NAV_SCREEN);
    }
}

void nextSettingArrowListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.RIGHT) {
        SETTINGS_SCREEN.incrementIndex();
    } else if (CONTROLS.arrows.getState() == CONTROLS.LEFT) {
        SETTINGS_SCREEN.decrementIndex();
    }
    if (CONTROLS.arrows.getState() == CONTROLS.DOWN) {
        SETTINGS_SCREEN.incrementTab();
    } else if (CONTROLS.arrows.getState() == CONTROLS.UP) {
        SETTINGS_SCREEN.decrementTab();
    }
}

void editSettingArrowListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.RIGHT) {
        SETTINGS_SCREEN.changeValue(50);
    } else if (CONTROLS.arrows.getState() == CONTROLS.LEFT) {
        SETTINGS_SCREEN.changeValue(-50);
    }
    if (CONTROLS.arrows.getState() == CONTROLS.DOWN) {
        SETTINGS_SCREEN.changeValue(500);
    } else if (CONTROLS.arrows.getState() == CONTROLS.UP) {
        SETTINGS_SCREEN.changeValue(-500);
    }
}

void editSettingButtonListener() { SETTINGS_SCREEN.startChange(); }

void changeSettingWheelListener() { SETTINGS_SCREEN.changeValue(CONTROLS.wheel.getDelta()); }

void saveFieldSettingButtonListener() { SETTINGS_SCREEN.saveChange(); }
void cancelFieldSettingButtonListener() { SETTINGS_SCREEN.cancelChange(); }

void resetFieldSettingButtonListener() {
    if (CONTROLS.button1.isPressed()) {
        SETTINGS_SCREEN.overwriteWithDefault();
    }
}

// 0:"MM/DD/YY HH:MM:SS" | 1:Month 2:"/" 3:Day 4:"/" 5:Year 6:" " 7: hour 8:":" 9: min 10:":" 11:Sec |
// 12: "Auto off - " | 13:off_min 14:" min without input" | 15: "Battery under " 16:minV 17:"v"|

class : public SettingField {

  private:
    int8_t month;
    bool modified = false;
    char buffer[3] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {
        month += delta;
        if (month > 12)
            month = month % 12;
        else if (month <= 0) {
            month = 12 + month % 12; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(getLocalHour(now), getLocalMinute(now), getLocalSecond(now), getLocalDay(now), month, getLocalYear(now));
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            month = getLocalMonth(microsSinceEpoch());
        }
        sprintf(buffer, "%02d", month);
        return buffer;
    };
} monthField;

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
class : public SettingField {

  private:
    int8_t day;
    bool modified = false;
    char buffer[3] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {

        uint8_t daysInMonth = monthDays[getLocalMonth(microsSinceEpoch()) - 1];

        day += delta;
        if (day > daysInMonth)
            day = day % daysInMonth;
        else if (day <= 0) {
            day = daysInMonth + day % daysInMonth; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(getLocalHour(now), getLocalMinute(now), getLocalSecond(now), day, getLocalMonth(now), getLocalYear(now));
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            day = getLocalDay(microsSinceEpoch());
        }
        sprintf(buffer, "%02d", day);
        return buffer;
    };
} dayField;

class : public SettingField {

  private:
    int16_t year;
    bool modified = false;
    char buffer[5] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {
        year += delta;
        if (year > 2099)
            year = 2000;
        else if (year < 2000) {
            year = 2099; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(getLocalHour(now), getLocalMinute(now), getLocalSecond(now), getLocalDay(now), getLocalMonth(now), year);
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            year = getLocalYear(microsSinceEpoch());
        }
        sprintf(buffer, "%04d", year);
        return buffer;
    };
} yearField;

class : public SettingField {

  private:
    int8_t hour;
    bool modified = false;
    char buffer[3] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {
        hour += delta;
        if (hour >= 24)
            hour = hour % 24;
        else if (hour < 0) {
            hour = 24 + hour % 24; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(hour, getLocalMinute(now), getLocalSecond(now), getLocalDay(now), getLocalMonth(now), getLocalYear(now));
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            hour = getLocalHour(microsSinceEpoch());
        }
        sprintf(buffer, "%02d", hour);
        return buffer;
    };
} hourField;

class : public SettingField {

  private:
    int8_t min;
    bool modified = false;
    char buffer[3] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {
        min += delta;
        if (min >= 60)
            min = min % 60;
        else if (min < 0) {
            min = 60 + min % 60; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(getLocalHour(now), min, getLocalSecond(now), getLocalDay(now), getLocalMonth(now), getLocalYear(now));
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            min = getLocalMinute(microsSinceEpoch());
        }
        sprintf(buffer, "%02d", min);
        return buffer;
    };
} minuteField;

class : public SettingField {

  private:
    int8_t sec;
    bool modified = false;
    char buffer[3] = {0};

  public:
    bool readOnly() { return false; };

    void modify(int delta) {

        sec += delta;
        if (sec >= 60)
            sec = sec % 60;
        else if (sec < 0) {
            sec = 60 + sec % 60; // remember negative mod result from negative val
        }
        modified = true;
    };

    void save() {
        if (!modified)
            return;
        TIME_INT_t now = microsSinceEpoch();
        setTimeManual(getLocalHour(now), getLocalMinute(now), sec, getLocalDay(now), getLocalMonth(now), getLocalYear(now));
        modified = false;
    };

    void reset() { modified = false; }

    const char *getText() {
        if (!modified) {
            sec = getLocalSecond(microsSinceEpoch());
        }
        sprintf(buffer, "%02d", sec);
        return buffer;
    };
} secondField;

LabelField slashLabel("\\");
LabelField spaceLabel(" ");
LabelField colonLabel(":");
LabelField dateTimeLabel("Date/Time - MM/DD/YY HH:MM:SS");
LabelField autoOffLabel("Auto off -");
LabelField offMinLabel(" minute(s) without input");
LabelField offBatteryLabel(" v battery");

SettingField *SettingsScreen::FIELDS[SettingsScreen::FIELD_COUNT] = {
    &dateTimeLabel, &monthField, &slashLabel,  &dayField,     &slashLabel,        &yearField,   &spaceLabel,      &hourField,       &colonLabel,
    &minuteField,   &colonLabel, &secondField, &autoOffLabel, &field_AutoOff_Min, &offMinLabel, &field_AutoOff_V, &offBatteryLabel, &slashLabel};

const uint8_t SettingsScreen::LINES[LINE_COUNT] = {0, 1, 12, 13, 15};
const uint8_t SettingsScreen::TABS[TAB_COUNT] = {0, 12};
const String SettingsScreen::TAB_NAMES[TAB_COUNT] = {"DATE & TIME", "AUTO OFF"};

String SettingsScreen::getField(uint8_t index, bool &editable) {
    if (index >= FIELD_COUNT) {
        editable = false;
        return "??";
    }
    editable = !FIELDS[index]->readOnly();
    return FIELDS[index]->getText();
}

void SettingsScreen::startChange() {
    if (editing)
        return;
    if (!editing) {
        if (FIELDS[index]->readOnly()) {
            return;
        }
        CONTROLS.arrows.unsubscribe(nextSettingArrowListener);
        CONTROLS.button5.unsubscribe(exitSettingsButtonListener);
        CONTROLS.button3.subscribe(cancelFieldSettingButtonListener);
        CONTROLS.button4.subscribe(saveFieldSettingButtonListener);
        CONTROLS.arrows.subscribe(editSettingArrowListener);
        CONTROLS.button3.setLEDValue(255);
        CONTROLS.button4.setLEDValue(255);
        CONTROLS.button5.setLEDValue(0);
        editing = true;
    }
}

void SettingsScreen::changeValue(int val) {

    if (!editing)
        return;

    partialValueChange += val % 2;

    if (partialValueChange % 2 == 0) {
        val += partialValueChange;
        partialValueChange = 0;
    }

    val = val / 2;

    if (val != 0 )
     FIELDS[index]->modify(val);
}

void SettingsScreen::cancelChange() {
    if (editing) {
        editing = false;
        FIELDS[index]->reset();
        exitEditMode();
    }
}

void SettingsScreen::saveChange() {
    if (editing) {
        editing = false;
        FIELDS[index]->save();
        exitEditMode();
    }
}

void SettingsScreen::overwriteWithDefault() { FIELDS[index]->overwriteWithDefault(); }

void SettingsScreen::exitEditMode() {
    CONTROLS.arrows.subscribe(nextSettingArrowListener);
    CONTROLS.button5.subscribe(exitSettingsButtonListener);
    CONTROLS.button3.unsubscribe(cancelFieldSettingButtonListener);
    CONTROLS.button4.unsubscribe(saveFieldSettingButtonListener);
    CONTROLS.arrows.unsubscribe(editSettingArrowListener);

    CONTROLS.button3.setLEDValue(0);
    CONTROLS.button4.setLEDValue(0);
    CONTROLS.button5.setLEDValue(255);
      CONTROLS.button1.setLEDValue(255* !FIELDS[index]->readOnly());
}

void SettingsScreen::incrementIndex() {
    index++;
    if (index == FIELD_COUNT)
        index = 0;
 CONTROLS.button1.setLEDValue(255* !FIELDS[index]->readOnly());
}

void SettingsScreen::decrementIndex() {
    if (index == 0)
        index = FIELD_COUNT;
    index--;
   CONTROLS.button1.setLEDValue(255* !FIELDS[index]->readOnly());
}

void SettingsScreen::incrementTab() {
    int line = getLine(index);
    line++;
    if (line == LINE_COUNT)
        line = 0;
    index = getIndexAtLine(line);
   CONTROLS.button1.setLEDValue(255* !FIELDS[index]->readOnly());
}

void SettingsScreen::decrementTab() {
    int line = getLine(index);
    line--;
    if (line == 0)
        line = LINE_COUNT - 1;
    index = getIndexAtLine(line);
   CONTROLS.button1.setLEDValue(255* !FIELDS[index]->readOnly());
}

uint8_t SettingsScreen::getLine(uint8_t index) {
    if (index == 0)
        return 0;
    for (uint8_t i = LINE_COUNT - 1; i >= 0; i--) {
        if (index >= LINES[i]) {
            return i;
        }
    }
    return 0;
}

uint8_t SettingsScreen::getIndexAtLine(uint8_t line) {
    if (line >= LINE_COUNT)
        return FIELD_COUNT - 1;
    return LINES[line];
}
void SettingsScreen::drawField(String &text, bool editable, bool selected) {
    UI.getDisplay()->setFont(editable ? u8g2_font_helvB14_tr : u8g2_font_helvR14_tr);
    uint16_t width = UI.getDisplay()->getStrWidth(text.c_str());
    uint16_t x = UI.getDisplay()->getCursorX() - 1;
    uint16_t y = UI.getDisplay()->getCursorY() - 2;
    if (selected) {
        if (!editable) {
            UI.getDisplay()->setDrawColor(0);
            UI.getDisplay()->drawFrame(x, y, width + 1, 20);
        } else {
            UI.getDisplay()->setDrawColor(0);
            UI.getDisplay()->drawBox(x, y, width + 1, 20);
            UI.getDisplay()->setDrawColor(1);
        }
    } else {
        UI.getDisplay()->setDrawColor(0);
    }
    UI.getDisplay()->print(text);
}

void SettingsScreen::start() {
    if (link == NULL) {
        CONTROLS.wheelBtn.subscribe(editSettingButtonListener);
        CONTROLS.button1.subscribe(resetFieldSettingButtonListener);
        CONTROLS.arrows.subscribe(nextSettingArrowListener);
        CONTROLS.wheel.subscribe(changeSettingWheelListener);
        CONTROLS.button5.subscribe(exitSettingsButtonListener);
        CONTROLS.button5.setLEDValue(255);
        link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(100, FrequencyUnitEnum::milli));
        index = 1;        // These two commands will trigger proper events when selecting the first field
        decrementIndex(); //
    }
}
void SettingsScreen::stop() {
    if (link != NULL) {
        CONTROLS.wheelBtn.unsubscribe(editSettingButtonListener);
        CONTROLS.button1.unsubscribe(resetFieldSettingButtonListener);
        CONTROLS.arrows.unsubscribe(nextSettingArrowListener);
        CONTROLS.wheel.unsubscribe(changeSettingWheelListener);
        CONTROLS.button5.unsubscribe(exitSettingsButtonListener);
        CONTROLS.button5.setLEDValue(0);
        link->cancel();
        link = NULL;
    }
}

void SettingsScreen::run(TIME_INT_t time) {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 217);
    uint8_t currentLine = getLine(index);
    uint8_t startLine = max(0, currentLine - 3);
    uint8_t endLine = min(LINE_COUNT - 1, startLine + 6);
    uint8_t startIndex = getIndexAtLine(startLine);
    uint8_t endIndex = endLine == LINE_COUNT - 1 ? FIELD_COUNT : (getIndexAtLine(endLine + 1));
    UI.getDisplay()->setCursor(4, 30);
    uint8_t printLine = startLine;
    for (int i = startIndex; i < endIndex; i++) {
        if (i == getIndexAtLine(printLine + 1)) {
            printLine++;
            UI.getDisplay()->setCursor(4, 30 + 26 * (printLine - startLine));
        }
        bool editable = false;
        String text = getField(i, editable);
        bool selected = i == index;
        drawField(text, editable, selected);
    }
    UI.requestDraw();
}

void FlightScreen::start() { CONTROLS.wheelBtn.subscribe(navEnableButtonListener); }
void FlightScreen::stop() { CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener); }

void FlightScreen::run(TIME_INT_t time) {}

void PCScreen::start() {
    if (link == NULL) {
        Joystick.useManualSend(true);
        Joystick.begin();
        CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
        link = EXECUTOR.schedule((RunnableTask *)this, 0, CONTROLS.getIntervalMicros());

        CONTROLS.joy1H.setHalfRange(512);
        CONTROLS.joy1V.setHalfRange(512);
        CONTROLS.joy2H.setHalfRange(512);
        CONTROLS.joy2V.setHalfRange(512);
    }
}
void PCScreen::stop() {
    if (link != NULL) {
        Joystick.end();
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
        link->cancel();
        link = NULL;
    }
}

void PCScreen::run(TIME_INT_t time) {
    updateJoystick();
    runCount++;
    if (runCount == 20) {
        runCount = 0;
        updateScreen();
    }
}

void PCScreen::updateJoystick() {
    CONTROLS.button1.setLEDValue(CONTROLS.button1.isPressed() * 255);
    CONTROLS.button2.setLEDValue(CONTROLS.button2.isPressed() * 255);
    CONTROLS.button3.setLEDValue(CONTROLS.button3.isPressed() * 255);
    CONTROLS.button4.setLEDValue(CONTROLS.button4.isPressed() * 255);
    CONTROLS.button5.setLEDValue(CONTROLS.button5.isPressed() * 255);

    Joystick.button(1, CONTROLS.button2.isPressed());
    Joystick.button(2, CONTROLS.button1.isPressed());
    Joystick.button(3, CONTROLS.button5.isPressed());
    Joystick.button(4, CONTROLS.button4.isPressed());
    Joystick.button(5, CONTROLS.button3.isPressed());
    switch (CONTROLS.arrows.getState()) {
    case 0:
        Joystick.hat(-1);
        break;
    case CONTROLS.LEFT:
        Joystick.hat(270);
        break;
    case CONTROLS.RIGHT:
        Joystick.hat(90);
        break;
    case CONTROLS.UP:
        Joystick.hat(0);
        break;
    case CONTROLS.DOWN:
        Joystick.hat(180);
        break;
    }
    wheelLoc += CONTROLS.wheel.getDelta() * 4;
    if (wheelLoc > 1023)
        wheelLoc = 1023;
    else if (wheelLoc < 0)
        wheelLoc = 0;
    Joystick.sliderRight(wheelLoc);
    Joystick.sliderLeft(CONTROLS.joy1V.getUnsignedValue());
    Joystick.Zrotate(CONTROLS.joy1H.getUnsignedValue());
    Joystick.X(CONTROLS.joy2H.getUnsignedValue());
    Joystick.Y(CONTROLS.joy2V.getUnsignedValue());
    Joystick.send_now();
}

void PCScreen::updateScreen() {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 217);
    drawXYInput(2, 26, 175, CONTROLS.joy2H.getUnsignedValue(), CONTROLS.joy2V.getUnsignedValue(), 1023, true);
    drawHInput(200, 35, 180, 20, CONTROLS.joy1H.getUnsignedValue(), 1023, true);

    drawVInput(210, 70, 120, 20, CONTROLS.joy1V.getUnsignedValue(), 1023, true);
    drawVInput(250, 70, 120, 20, wheelLoc, 1023, true);

    UI.getDisplay()->drawCircle(330, 130, 40);
    switch (CONTROLS.arrows.getState()) {
    case 0:
        break;
    case CONTROLS.LEFT:
        UI.getDisplay()->drawTriangle(292, 130, 302, 125, 302, 135);
        break;
    case CONTROLS.RIGHT:
        UI.getDisplay()->drawTriangle(368, 130, 358, 125, 358, 135);
        break;
    case CONTROLS.UP:
        UI.getDisplay()->drawTriangle(330, 92, 325, 102, 335, 102);
        break;
    case CONTROLS.DOWN:
        UI.getDisplay()->drawTriangle(330, 168, 325, 158, 335, 158);
        break;
    }

    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawDisc(200, 212, 6);
    UI.getDisplay()->drawCircle(200, 212, 15);
    UI.getDisplay()->drawCircle(200, 212, 20);
    UI.getDisplay()->drawHLine(209, 212, 18);
    UI.getDisplay()->setFont(u8g2_font_10x20_mr);
    UI.getDisplay()->drawStr(230, 205, "Nav");
    UI.requestDraw();
}