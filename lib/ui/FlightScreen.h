#ifndef flight_screen_H__
#define flight_screen_H__

#include "Screens.h"

class FlightScreen : Screen, RunnableTask {
  private:
    String title = "Flight Control";

    enum FlightScreenState { OFF, SEARCHING, CONNECTED, ENGAGED, CONFIG, PAUSED } state = OFF;

    ScheduledLink *link = NULL;

    bool telemCaptureEnabled = false;

    void drawInputs();
    void drawReceiverStats();

  public:
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    void toggleTelem();
    void toggleRadio();
    void toggleEngage();
    void startConfig();
    void exitEvent();
};

class FlightConfigScreen : Screen, RunnableTask {
  private:
    static const int8_t maxSelection = 16;

    static const uint16_t PID_FIELD_COUNT = 12;

    SettingField *PID_FIELDS[PID_FIELD_COUNT];

    String title = "Flight Config";

    ScheduledLink *link = NULL;

    void drawESC();
    void drawPID();

    int16_t partialValueChange = 0;

    uint8_t selection = 0;
    bool editing = false;

    int8_t runtime = 5;
    uint8_t escVals[4] = {0};

    void setupEditMode();
    void exitEditMode();
    void setupSelectMode();
    void exitSelectMode();

  public:
    FlightConfigScreen();
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    void sendESC();
    void sendPIDSettings();

    void moveSelection(int8_t move);
    void startChange();
    void saveChange();
    void cancelChange();
    void resetValue();
    void changeValue(int16_t value);
};

class FlightTelemScreen : Screen, RunnableTask {
  private:
    static const int8_t maxSelection = 16;

    static const uint16_t PID_FIELD_COUNT = 12;

    SettingField *PID_FIELDS[PID_FIELD_COUNT];

    String title = "Flight Telemetry";

    ScheduledLink *link = NULL;

    uint16_t selection = 0;
    uint16_t sampleCount = 0;
    bool loadComplete = 0;

    int16_t partialValueChange = 0;

  public:
    void start();
    void stop();
    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    void exit();
    void scroll(int16_t change);
    void toTop();
    void toBottom();
    void dumpToSerial();
};

extern FlightScreen FLIGHT_SCREEN;
extern FlightConfigScreen FLIGHT_CONFIG_SCREEN;
extern FlightTelemScreen FLIGHT_TELEM_SCREEN;

#endif