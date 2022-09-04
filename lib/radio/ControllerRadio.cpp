#include <ControllerRadio.h>

FindReceiverAction radioFindReceiverAction = FindReceiverAction();
SustainConnectionAction sustainConnectionAction = SustainConnectionAction();

void FindReceiverAction::onStart() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(5, FrequencyUnitEnum::second));
    state = PINGING;
}

void FindReceiverAction::onStop() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = NULL;
}

void FindReceiverAction::onReceive(uint8_t length, uint8_t *data, bool responseExpected) {
    switch (state) {
    case PINGING:
        if (data[0] == RADIO_MSG_ID::RECEIVER_RECOGNIZE) {
            if (data[2] != TRANSMITTER_ID) { // Wrong receiver ID returned, conflicting devices - stop connecting
                state = OTHER_TRANSMITTER;
#ifdef SND_OUTPUT_PIN
                tone(SND_OUTPUT_PIN, 800, 2000);
#endif
                analogWrite(LED_R_PIN, 255);
                RADIOTASK.removeAction(this);
                return;
            }
            state = CONNECTING;
            receiverId = data[1];
#ifdef SND_OUTPUT_PIN
            tone(SND_OUTPUT_PIN, 2300, 800);
#endif
            cancel->cancel();
            cancel = EXECUTOR.schedule((RunnableTask *)this, 1000000, 1000000); // Wait 1 second before sending back the confirmation
        }
        break;

    default:
        break;
    }
}

void FindReceiverAction::run(TIME_INT_t time) {
    switch (state) {
    case PINGING:
#ifdef SND_OUTPUT_PIN
        tone(SND_OUTPUT_PIN, 4000, 100);
#endif
        analogWrite(LED_R_PIN, 200);
        requestSend();
        break;
    case CONNECTING:
        requestSend();
        break;
    default:
        break;
    }
}

uint8_t FindReceiverAction::onSendReady(uint8_t *data, bool &responseExpected) {
    switch (state) {
    case PINGING:
        data[0] = RADIO_MSG_ID::TRANSMITTER_BEACON;
        data[1] = TRANSMITTER_ID;
        responseExpected = true;
        analogWrite(LED_R_PIN, 0);
        return 2;
    case CONNECTING:
        data[0] = RADIO_MSG_ID::TRANSMITTER_RECOGNIZE_CONFIRM;
        data[1] = TRANSMITTER_ID;
        data[2] = receiverId;
        state = CONNECTED;
        RADIOTASK.removeAction(this);
        RADIOTASK.addAction((RadioAction *)&sustainConnectionAction);
        return 3;
    default:
        return 0;
        break;
    }
}

void SustainConnectionAction::onStart() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(TRANSMITTER_HB_MAX_MICROS, FrequencyUnitEnum::micro));
    connected = true;
}

void SustainConnectionAction::onStop() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = NULL;
}

void SustainConnectionAction::run(TIME_INT_t time) {
    if (!connected) {
        RADIOTASK.removeAction(this);
        return;
    }
    if (time - lastContactSentTime > TRANSMITTER_HB_MIN_MICROS) { // there was no proactive HB
        requestSend();
        lastContactSentTime = time;
        return;
    }
    if (time - lastContactReceivedTime > TRANSMITTER_HB_ABANDON_MICROS){
        disconnect();
    }
}

void SustainConnectionAction::onReceive(uint8_t length, uint8_t *data, bool responseExpected) {
    if (data[0] == RECEIVER_HEARTBEAT) {
        lastContactReceivedTime = microsSinceEpoch();
        msgFromBytes(&lastReceivedHB, data + 1, receiver_heartbeat_t::size);
        return;
    }
}

bool SustainConnectionAction::shouldRequestResponse() {
    TIME_INT_t time = microsSinceEpoch();
    if (time - lastContactSentTime > TRANSMITTER_HB_MIN_MICROS) {
        lastContactSentTime = time;
        RADIOTASK.mute(TRANSMITTER_HB_ECHO_DELAY_MILLIS);
        return true;
    }
    return false;
}

uint8_t SustainConnectionAction::onSendReady(uint8_t *data, bool &responseExpected) {
    if (connected) {
        responseExpected = true;
        data[0] = TRANSMITTER_HEARTBEAT;
        data[1] = motorsEngaged;
        RADIOTASK.mute(TRANSMITTER_HB_ECHO_DELAY_MILLIS);
        return 2;
    } else {
        data[0] = TRANSMITTER_DISCONNECT;
        data[1] = TRANSMITTER_ID;
        return 2;
    }
}

TIME_INT_t SustainConnectionAction::getLastReceivedHB(receiver_heartbeat_t &hb) {
    hb = lastReceivedHB;
    return lastContactReceivedTime;
}

void SustainConnectionAction::disconnect() {
    if (!connected)
        return;
    connected = false;
    requestSend();
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this,
                               EXECUTOR.getTimingPair(RADIO_INTERVAL_MILLIS * 2, FrequencyUnitEnum::milli)); // Reschedule to shut down after disconnect is sent
}


