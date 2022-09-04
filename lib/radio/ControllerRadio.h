#ifndef __controller_radio_H__
#define __controller_radio_H__

#include "RadioTask.h"
#include "VMExecutor.h"

class FindReceiverAction : RadioAction, RunnableTask {
  private:
    ScheduledLink *cancel = NULL;
    uint8_t receiverId = 0;

  public:
    enum { PINGING, CONNECTING, CONNECTED, OTHER_TRANSMITTER } state;

    void onStart();

    void onStop();

    void onReceive(uint8_t length, uint8_t *data, bool responseExpected);

    uint8_t onSendReady(uint8_t *data, bool &responseExpected);

    void run(TIME_INT_t time);
};

class SustainConnectionAction : RadioAction, RunnableTask {
  private:
    ScheduledLink *cancel = NULL;
    TIME_INT_t lastContactSentTime = 0;
    bool inHBRange = false;

    receiver_heartbeat_t lastReceivedHB;
    TIME_INT_t lastContactReceivedTime = 0;

    bool connected = false;

    bool motorsEngaged = false;

  public:
    bool shouldRequestResponse();

    void onStart();

    void onStop();

    void onReceive(uint8_t length, uint8_t *data, bool responseExpected);

    uint8_t onSendReady(uint8_t *data, bool &responseExpected);

    void run(TIME_INT_t time);

    TIME_INT_t getLastReceivedHB(receiver_heartbeat_t &hb);

    bool isConnected() { return connected; }

    void disconnect();

    void setEngineEngaged(bool engaged) { motorsEngaged = engaged; }
};

extern FindReceiverAction radioFindReceiverAction;
extern SustainConnectionAction sustainConnectionAction;

#endif