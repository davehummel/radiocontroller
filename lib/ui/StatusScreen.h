#ifndef status_screens_H__
#define status_screens_H__

#include "Screens.h"

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

#endif