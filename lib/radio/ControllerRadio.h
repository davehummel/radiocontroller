#ifndef __controller_radio_H__
#define __controller_radio_H__

#include "RadioTask.h"
#include "VMExecutor.h"


class FindReceiverAction : RadioAction, RunnableTask {
  private:
    ScheduledLink *cancel = NULL;
    uint8_t receiverId = 0;

  public:

    enum {PINGING,CONNECTING, CONNECTING_CONFIRM_WAIT,CONNECTED,OTHER_TRANSMITTER} state;

    void onStart();

    void onStop();

    void onReceive(uint8_t length, uint8_t *data);

    uint8_t onSendReady(uint8_t *data, bool &responseExpected);

    void run(TIME_INT_t time);

};

extern FindReceiverAction radioFindReceiverTask;

#endif