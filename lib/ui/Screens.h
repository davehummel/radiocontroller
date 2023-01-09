#ifndef screens_H__
#define screens_H__
#include "RemoteControlInputs.h"
#include "RemoteDisplay.h"
#include "RootUI.h"
#include "SettingsStore.h"
#include "VMExecutor.h"
#include "VMTime.h"

class NavScreen : Screen, RunnableTask {
  private:
    String title = "Navigation";
    ScheduledLink *link = NULL;

    void drawGuide();

  public:
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }
};

class SettingsScreen : Screen, RunnableTask {
  private:
    static const uint16_t FIELD_COUNT = 71;
    static const uint8_t LINE_COUNT = 19;
    static const uint8_t TAB_COUNT = 2;

    static const uint8_t LINES[LINE_COUNT];
    static const uint8_t TABS[TAB_COUNT];
    static const String TAB_NAMES[TAB_COUNT];

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

class FlightScreen : Screen, RunnableTask {
  private:
    String title = "Flight Control";

    enum FlightScreenState { OFF, SEARCHING, CONNECTED , ENGAGED,PAUSED } state = OFF;

    ScheduledLink *link = NULL;

    void drawInputs();
    void drawReceiverStats();

  public:
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    
    void toggleRadio();
    void toggleEngage();
};

class PCScreen : Screen, RunnableTask {
  private:
    String title = "PC Control";

    int wheelLoc = 512;
    uint8_t runCount = 0;

    ScheduledLink *link = NULL;

    void updateJoystick();
    void updateScreen();

  public:
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }
};

class StatusScreen : Screen, RunnableTask {
  private:
    String title = "Status";

    ScheduledLink *link = NULL;

    uint8_t page;

    void updateRootUI();

    void renderSummary();

    void renderInput();

    void renderConfig();

    void renderPage();

    void drawMenu(const char *text, bool isLeft);

  public:
    void start();

    void stop();

    void run(TIME_INT_t time);

    void pageRight();

    void pageLeft();

    String &getTitle() { return title; }
};

extern StatusScreen STATUS_SCREEN;
extern NavScreen NAV_SCREEN;
extern SettingsScreen SETTINGS_SCREEN;
extern FlightScreen FLIGHT_SCREEN;
extern PCScreen PC_SCREEN;

#endif
