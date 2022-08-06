#ifndef remote__display_H__
#define remote__display_H__
#include <U8g2lib.h>
#include "VMExecutor.h"

class RemoteUI : RunnableTask {

  private:
    U8G2 *display;
    bool redrawNeeded = false;
    bool redrawMissed = false;

    bool spiLock = false;

  public:
    RemoteUI(U8G2* display):display(display){}
    void run(TIME_INT_t time);
    void requestDraw(bool immediate);
    U8G2* getDisplay(){
      return display;
    }

    void setSPIBusy();
    void setSPIFree();

};

#endif