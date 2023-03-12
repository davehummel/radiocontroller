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

    bool sharedPIDConfig = false;

    bool motorsEngaged = false;

    bool directPitch = true, directYaw = true, directRoll = true;

    char errorMessage[255] = "\0";

    direct_esc_t directESC;

  public:
    bool shouldRequestResponse();

    void onStart();

    void onStop();

    void onReceive(uint8_t length, uint8_t *data, bool responseExpected);

    void onCriticalMessage(char *data);

    uint8_t onSendReady(uint8_t *data, bool &responseExpected);

    void run(TIME_INT_t time);

    TIME_INT_t getLastReceivedHB(receiver_heartbeat_t &hb);

    bool isConnected() { return connected; }

    void disconnect();

    void setEngineEngaged(bool engaged);

    void setDirectPitch(bool directEnabled) { directPitch = directEnabled; requestSend(); }
    void setDirectYaw(bool directEnabled) { directYaw = directEnabled; requestSend();}
    void setDirectRoll(bool directEnabled) { directRoll = directEnabled; requestSend();}

    bool getDirectPitch() { return directPitch; }
    bool getDirectYaw() { return directYaw; }
    bool getDirectRoll() { return directRoll; }

    void setESC(uint8_t runtimeSeconds,uint8_t* escVals);
};

class TransmitCommandAction : RadioAction, RunnableTask {
  private:
    ScheduledLink *cancel = NULL;

    bool inputChanged = true;

  public:
    void onStart();

    void onStop();

    void onReceive(uint8_t length, uint8_t *data, bool responseExpected);

    uint8_t onSendReady(uint8_t *data, bool &responseExpected);

    void run(TIME_INT_t time);

    void changed() { inputChanged = true; }
};

extern FindReceiverAction radioFindReceiverAction;
extern SustainConnectionAction sustainConnectionAction;
extern TransmitCommandAction transmitCommandAction;

#endif