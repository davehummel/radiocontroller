#include <ControllerRadio.h>

FindReceiverAction radioFindReceiverTask = FindReceiverAction();

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

void FindReceiverAction::onReceive(uint8_t length, uint8_t *data) {
    switch (state) {
    case PINGING:
        if (data[0] == RADIO_MSG_ID::RECEIVER_RECOGNIZE) {
            if (data[2] != TRANSMITTER_ID) {
                state = OTHER_TRANSMITTER;
#ifdef SND_OUTPUT_PIN
                tone(SND_OUTPUT_PIN, 800, 500);
#endif
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
    case CONNECTING_CONFIRM_WAIT:
        if (data[0] == RADIO_MSG_ID::RECEIVER_RECOGNIZE_CONFIRM) {
            if (data[2] == TRANSMITTER_ID && data[1] == receiverId) {
                state = CONNECTED;
#ifdef SND_OUTPUT_PIN
                tone(SND_OUTPUT_PIN, 2600, 400);
#endif
                RADIOTASK.removeAction(this);
                return;
                // START sustain connection radio task
            }
        }
    default:
        break;
    }
}

void FindReceiverAction::run(TIME_INT_t time) {
    switch (state) {
    case PINGING:
#ifdef SND_OUTPUT_PIN
        tone(SND_OUTPUT_PIN, 2000);
#endif
        requestSend();

        break;
    case CONNECTING:
        state = CONNECTING_CONFIRM_WAIT;
        requestSend();
        break;
    case CONNECTING_CONFIRM_WAIT:
#ifdef SND_OUTPUT_PIN
        tone(SND_OUTPUT_PIN, 400, 300);
#endif
        state = PINGING;
        onStart();
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
        #ifdef SND_OUTPUT_PIN
        noTone(SND_OUTPUT_PIN);
#endif
        return 2;
    case CONNECTING:
        data[0] = RADIO_MSG_ID::TRANSMITTER_RECOGNIZE_CONFIRM;
        data[1] = TRANSMITTER_ID;
        data[2] = receiverId;
        responseExpected = true;
        return 3;
    default:
        return 0;
        break;
    }
}