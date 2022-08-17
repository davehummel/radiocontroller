#ifndef remote__display_H__
#define remote__display_H__
#include "VMExecutor.h"
#include <U8g2lib.h>

class RemoteUI : RunnableTask {

  private:
    U8G2 *display;
    bool redrawNeeded = false;
    bool redrawMissed = false;

    bool spiLock = false;

    bool illuminated = false;

  public:
    void setDisplay(U8G2 *_display) { display = _display; }
    void start();
    void run(TIME_INT_t time);
    void requestDraw(bool immediate = false);
    U8G2 *getDisplay() { return display; }

    void setSPIBusy();
    void setSPIFree();

    void illuminate(bool on);
    bool isIlluminated();
};

extern RemoteUI UI;

#endif