#include "SettingScreen.h"

// 0:"MM/DD/YY HH:MM:SS" | 1:Month 2:"/" 3:Day 4:"/" 5:Year 6:" " 7: hour 8:":" 9: min 10:":" 11:Sec |
// 12: "Auto off - " | 13:off_min 14:" min without input" |  16:minV 17:"v min battery"|

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

void editSettingButtonListener() {
        SETTINGS_SCREEN.startChange();
}

void changeSettingWheelListener() { SETTINGS_SCREEN.changeValue(CONTROLS.wheel.getDelta()); }

void saveFieldSettingButtonListener() { SETTINGS_SCREEN.saveChange(); }
void cancelFieldSettingButtonListener() { SETTINGS_SCREEN.cancelChange(); }

void resetFieldSettingButtonListener() {
        SETTINGS_SCREEN.overwriteWithDefault();
}

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
        // CONTROLS.lastInputMicros(true);
        modified = false;
    };

    void cancel() { modified = false; }

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
LabelField dateTimeLabel("# Date/Time #");
LabelField autoOffLabel("# Auto off #");
LabelField offMinLabel(" minute(s) without input");
LabelField offBatteryLabel(" v battery minimum");
LabelField spacerLabel("# Inputs #");
LabelField inputSettingsLabel("Joystick Trim");
LabelField leftLabel(" <- ");
LabelField rightLabel("   -> ");
LabelField leftBracketLabel("[");
LabelField leftCarrotLabel("<");
LabelField lineLabel("|");
LabelField rightCarrotLabel(">");
LabelField rightBracketLabel("]");
LabelField radioLabel("# Radio #");
LabelField radioFreqLabel("Carier Freq ");
LabelField radioMHZLabel(" mhz");
LabelField radioBWLabel("Link BW (0->125,1->250,2->500khz) ");
LabelField radioSpreadLabel("Spread Factor (6-12) ");
LabelField radioCodingRateLabel("Coding Rate (5-8) ");
LabelField radioPowerLabel("Tx Power (2-20 dBm) ");
LabelField flightPIDLabel("#PID   KP   KD   KI    MAXI");
LabelField flightYAWLabel("  YAW:  ");
LabelField flightPIDSpace("   ");
LabelField flightROLLLabel(" ROLL:  ");
LabelField flightPITCHLabel("PITCH:  ");

class JoyStatsField : public SettingField {
  private:
    int hInput, vInput;
    char buffer[32] = {0};

  public:
    JoyStatsField(int hPin, int vPin) : hInput(hPin), vInput(vPin) {}

    bool readOnly() { return true; }
    const char *getText() {
        sprintf(buffer, "H %04i V %04i", analogRead(hInput), analogRead(vInput));
        return buffer;
    }
};

JoyStatsField joy2StatsLabel(JOY2_H_PIN, JOY2_V_PIN);
JoyStatsField joy1StatsLabel(JOY1_H_PIN, JOY1_V_PIN);

const uint8_t SettingsScreen::LINES[LINE_COUNT] = {0, 1, 12, 13, 15, 17, 18, 19, 21, 30, 39, 41, 50, 59, 60, 63, 65, 67, 69, 71, 72, 80, 88};

SettingsScreen::SettingsScreen()
    : FIELDS{
          &dateTimeLabel,
          &monthField,
          &slashLabel,
          &dayField,
          &slashLabel,
          &yearField,
          &spaceLabel,
          &hourField,
          &colonLabel, // 9
          &minuteField,
          &colonLabel,
          &secondField,
          &autoOffLabel,
          &field_AutoOff_Min,
          &offMinLabel,
          &field_AutoOff_V,
          &offBatteryLabel,
          &spacerLabel, // 18
          &inputSettingsLabel,
          &leftLabel,
          &joy2StatsLabel,
          &leftBracketLabel,
          &field_joy2H_Min,
          &leftCarrotLabel,
          &field_joy2H_Mid1,
          &lineLabel,
          &field_joy2H_Mid2, // 27
          &rightCarrotLabel,
          &field_joy2H_Max,
          &rightBracketLabel,
          &leftBracketLabel,
          &field_joy2V_Min,
          &leftCarrotLabel,
          &field_joy2V_Mid1,
          &lineLabel,
          &field_joy2V_Mid2, // 36
          &rightCarrotLabel,
          &field_joy2V_Max,
          &rightBracketLabel,
          &joy1StatsLabel,
          &rightLabel,
          &leftBracketLabel,
          &field_joy1H_Min,
          &leftCarrotLabel,
          &field_joy1H_Mid1,
          &lineLabel, // 45
          &field_joy1H_Mid2,
          &rightCarrotLabel,
          &field_joy1H_Max,
          &rightBracketLabel,
          &leftBracketLabel,
          &field_joy1V_Min,
          &leftCarrotLabel,
          &field_joy1V_Mid1,
          &lineLabel, // 54
          &field_joy1V_Mid2,
          &rightCarrotLabel,
          &field_joy1V_Max,
          &rightBracketLabel,
          &radioLabel,
          &radioFreqLabel, // 60
          &field_radio_Freq,
          &radioMHZLabel,
          &radioBWLabel,
          &field_radio_Linkbw,
          &radioSpreadLabel,
          &field_radio_SpreadingFactor, // 66
          &radioCodingRateLabel,
          &field_radio_CodingRate,
          &radioPowerLabel,
          &field_radio_Power, // 70
          &flightPIDLabel,    // 71
          &flightYAWLabel,
          &field_PID_yaw_kp,
          &flightPIDSpace,
          &field_PID_yaw_ki,
          &flightPIDSpace,
          &field_PID_yaw_kd,
          &flightPIDSpace,
          &field_PID_yaw_max_i, // 79
          &flightROLLLabel,
          &field_PID_roll_kp,
          &flightPIDSpace,
          &field_PID_roll_ki,
          &flightPIDSpace,
          &field_PID_roll_kd,
          &flightPIDSpace,
          &field_PID_roll_max_i, // 87
          &flightPITCHLabel,
          &field_PID_pitch_kp,
          &flightPIDSpace,
          &field_PID_pitch_ki,
          &flightPIDSpace,
          &field_PID_pitch_kd,
          &flightPIDSpace,
          &field_PID_pitch_max_i, // 95
      } {}

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
        Serial.printf("Editing %p with val %s\n", FIELDS[index], FIELDS[index]->getText());
        CONTROLS.arrows.unsubscribe(nextSettingArrowListener);
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
        CONTROLS.button3.subscribe(cancelFieldSettingButtonListener);
        CONTROLS.button4.subscribe(saveFieldSettingButtonListener);
        CONTROLS.arrows.subscribe(editSettingArrowListener);
        CONTROLS.button3.setLEDValue(255);
        CONTROLS.button4.setLEDValue(255);
        CONTROLS.button5.setLEDValue(0);

        drawNavMenu(EMPTY_TITLE, "Save", "Cancel", EMPTY_TITLE, EMPTY_TITLE, "Reset");

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

    if (val != 0)
        FIELDS[index]->modify(val);
}

void SettingsScreen::cancelChange() {
    if (editing) {
        editing = false;
        FIELDS[index]->cancel();
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
    CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
    CONTROLS.button3.unsubscribe(cancelFieldSettingButtonListener);
    CONTROLS.button4.unsubscribe(saveFieldSettingButtonListener);
    CONTROLS.arrows.unsubscribe(editSettingArrowListener);

    updateButtonLights();
}

void SettingsScreen::incrementIndex() {
    index++;
    if (index == FIELD_COUNT)
        index = 0;
    updateButtonLights();
}

void SettingsScreen::decrementIndex() {
    if (index == 0)
        index = FIELD_COUNT;
    index--;
    updateButtonLights();
}

void SettingsScreen::incrementTab() {
    int line = getLine(index);
    line++;
    if (line == LINE_COUNT)
        line = 0;
    index = getIndexAtLine(line);
    updateButtonLights();
}

void SettingsScreen::decrementTab() {
    int line = getLine(index);
    line--;
    if (line == 0)
        line = LINE_COUNT - 2;
    index = getIndexAtLine(line);
    updateButtonLights();
}

void SettingsScreen::updateButtonLights() {
    CONTROLS.button1.setLEDValue(255 * !FIELDS[index]->readOnly());
    CONTROLS.button2.setLEDValue(255 * !FIELDS[index]->readOnly());
    CONTROLS.button5.setLEDValue(0);
    CONTROLS.button3.setLEDValue(0);
    CONTROLS.button4.setLEDValue(0);

    drawNavMenu(EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, "Exit", FIELDS[index]->readOnly() ? EMPTY_TITLE : "Edit",
                FIELDS[index]->readOnly() ? EMPTY_TITLE : "Reset");
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
        return FIELD_COUNT;
    return LINES[line];
}
void SettingsScreen::drawField(String &text, bool editable, bool selected) {
    if (text.c_str()[0] == '#') {
        UI.getDisplay()->setFont(u8g2_font_chargen_92_tr);
    } else {
        UI.getDisplay()->setFont(editable ? u8g2_font_helvB14_tr : u8g2_font_helvR14_tr);
    }
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
        CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
        CONTROLS.button1.subscribe(resetFieldSettingButtonListener);
        CONTROLS.arrows.subscribe(nextSettingArrowListener);
        CONTROLS.wheel.subscribe(changeSettingWheelListener);
        CONTROLS.button2.subscribe(editSettingButtonListener);
        link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(100, FrequencyUnitEnum::milli));

        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 25, 400, 215);

        index = 1;        // These two lines will trigger proper events when selecting the first field
        decrementIndex(); //
    }
}
void SettingsScreen::stop() {
    if (link != NULL) {
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
        CONTROLS.button1.unsubscribe(resetFieldSettingButtonListener);
        CONTROLS.arrows.unsubscribe(nextSettingArrowListener);
        CONTROLS.wheel.unsubscribe(changeSettingWheelListener);
        CONTROLS.button2.unsubscribe(editSettingButtonListener);
        CONTROLS.button2.setLEDValue(0);
        link->cancel();
        link = NULL;
    }
}

void SettingsScreen::run(TIME_INT_t time) {
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 155);
    uint8_t currentLine = getLine(index);
    uint8_t startLine = max(0, currentLine - 3);
    uint8_t endLine = min(LINE_COUNT - 1, startLine + 6);
    uint8_t startIndex = getIndexAtLine(startLine);
    uint8_t endIndex = endLine == LINE_COUNT - 1 ? FIELD_COUNT : (getIndexAtLine(endLine + 1));
    UI.getDisplay()->setCursor(20, 27);
    uint8_t printLine = startLine;
    for (int i = startIndex; i < endIndex; i++) {
        if (i == getIndexAtLine(printLine + 1)) {
            printLine++;
            uint16_t y = 27 + 21 * (printLine - startLine);
            UI.getDisplay()->setCursor(20, y);
            if (printLine == currentLine) {
                UI.getDisplay()->setDrawColor(0);
                UI.getDisplay()->drawTriangle(0, y, 18, y + 8, 0, y + 16);
            }
        }
        bool editable = false;
        String text = getField(i, editable);
        bool selected = i == index;
        drawField(text, editable, selected);
    }
    UI.requestDraw();
}

SettingsScreen SETTINGS_SCREEN;