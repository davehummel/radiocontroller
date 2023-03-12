#ifndef setting_screen_H__
#define setting_screen_H__

#include "Screens.h"


class SettingsScreen : Screen, RunnableTask {
  private:
    static const uint16_t FIELD_COUNT = 96;
    static const uint8_t LINE_COUNT = 23;

    static const uint8_t LINES[LINE_COUNT];

    SettingField *FIELDS[SettingsScreen::FIELD_COUNT];

    uint8_t index = 0;

    bool editing = false;

    int8_t partialValueChange = 0;

    String title = "Settings";

    ScheduledLink *link = NULL;

    uint8_t getLine(uint8_t index);
    uint8_t getIndexAtLine(uint8_t index);

    String getField(uint8_t index, bool &editable);

    void exitEditMode();

    void drawField(String &text, bool editable, bool selected);

    void render(bool isEditable, bool isSelected, String text);

  public:
    SettingsScreen();
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    void incrementIndex();
    void decrementIndex();

    void incrementTab();
    void decrementTab();

    void startChange();

    void changeValue(int val);

    void cancelChange();

    void saveChange();

    void overwriteWithDefault();

    void updateButtonLights();
};

extern SettingsScreen SETTINGS_SCREEN;

#endif