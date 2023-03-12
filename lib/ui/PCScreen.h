#ifndef pcscreen_screen_H__
#define pcscreen_screen_H__

#include "Screens.h"



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

extern PCScreen PC_SCREEN;


#endif
