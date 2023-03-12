#ifndef flight_screen_H__
#define flight_screen_H__

#include "Screens.h"

class FlightScreen : Screen, RunnableTask {
  private:
    String title = "Flight Control";

    enum FlightScreenState { OFF, SEARCHING, CONNECTED , ENGAGED, CONFIG, PAUSED } state = OFF;

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
    void startConfig();
};

class FlightConfigScreen : Screen, RunnableTask {
  private:
    static const int8_t maxSelection = 4;

    String title = "Flight Config";


    ScheduledLink *link = NULL;

    void drawESC();
    void drawPID();
    void drawTelemetryView();
    void drawTelemetryRequest();

    int32_t partialValueChange = 0;

    uint8_t selection = 0;

    int8_t runtime = 5;
    uint8_t escVals [4] = {0};

  public:
    void start();
    void stop();

    void run(TIME_INT_t time);

    String &getTitle() { return title; }

    void toggleTelemetry();
    void exitConfig();
    void moveSelection(int8_t move);
    void changeValue(int8_t value);
    void sendESC();
};


extern FlightScreen FLIGHT_SCREEN;
extern FlightConfigScreen FLIGHT_CONFIG_SCREEN;


#endif