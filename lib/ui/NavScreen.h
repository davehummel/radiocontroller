#ifndef nav_screen_H__
#define nav_screen_H__

#include "Screens.h"

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

extern NavScreen NAV_SCREEN;

#endif