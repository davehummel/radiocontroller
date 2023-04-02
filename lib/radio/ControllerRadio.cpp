#include <ControllerRadio.h>
#include <RemoteControlInputs.h>

FindReceiverAction radioFindReceiverAction = FindReceiverAction();
SustainConnectionAction sustainConnectionAction = SustainConnectionAction();
TransmitCommandAction transmitCommandAction = TransmitCommandAction();
TelemetryAction telemetryAction = TelemetryAction();

void FindReceiverAction::onStart() {
    FDOS_LOG.println("FindReceiverAction Start");
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(5, FrequencyUnitEnum::second));
    state = PINGING;
}

void FindReceiverAction::onStop() {
    FDOS_LOG.println("FindReceiverAction Stop");
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
                FDOS_LOG.println("Received unrecognized response, abandoning connection");
                return;
            }
            state = CONNECTING;
            receiverId = data[1];
#ifdef SND_OUTPUT_PIN
            tone(SND_OUTPUT_PIN, 2300, 800);
#endif
            cancel->cancel();
            cancel = EXECUTOR.schedule((RunnableTask *)this, 1000000, 1000000); // Wait 1 second before sending back the confirmation
            FDOS_LOG.println("Received resoponse, waiting 1 second to confirm");
        }
        break;

    default:
        break;
    }
}

bool pingLight = false;
void FindReceiverAction::run(TIME_INT_t time) {
    switch (state) {
    case PINGING:
#ifdef SND_OUTPUT_PIN
        tone(SND_OUTPUT_PIN, 8000, 100);
#endif
        analogWrite(LED_R_PIN, 200 * (pingLight = !pingLight));
        FDOS_LOG.print("Pinging");
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
        FDOS_LOG.println("...");
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
        FDOS_LOG.println("Sent confirmation to compelete handshake");
        return 3;
    default:
        return 0;
        break;
    }
}

void SustainConnectionAction::onStart() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this, 10000, TRANSMITTER_HB_MAX_MICROS);
    lastContactReceivedTime = microsSinceEpoch();

    FDOS_LOG.println("Sustain connection starting...");
    connected = true;
    motorsEngaged = false;
    sharedPIDConfig = false;

    requestSend();
}

void SustainConnectionAction::onStop() {
    if (connected)
        disconnect();

    if (cancel != NULL)
        cancel->cancel();
    cancel = NULL;

    FDOS_LOG.println("Sustain Connection stopped.");
}

void SustainConnectionAction::run(TIME_INT_t time) {
    if (!connected) {
        FDOS_LOG.print("Not connected on HB run...");
        RADIOTASK.removeAction(this);
        return;
    }
    if (time - lastContactReceivedTime > TRANSMITTER_HB_ABANDON_MICROS) {
        FDOS_LOG.println("Disconnecting due to lack of HB");
        disconnect();
        RADIOTASK.removeAction(this);
        return;
    }
    if (time - lastContactSentTime > TRANSMITTER_HB_MIN_MICROS) { // there was no proactive HB
        FDOS_LOG.println("Sending HB");
        analogWrite(LED_R_PIN, 2);
        analogWrite(LED_B_PIN, 2);
        analogWrite(LED_G_PIN, 255);
        tone(SND_OUTPUT_PIN, 1000, 50);
        requestSend();
        lastContactSentTime = time;
        return;
    }
}

void SustainConnectionAction::onReceive(uint8_t length, uint8_t *data, bool responseExpected) {
    if (data[0] == RECEIVER_HEARTBEAT) {
        FDOS_LOG.println("HB response from receiver");
        tone(SND_OUTPUT_PIN, 2000, 50);
        lastContactReceivedTime = microsSinceEpoch();
        sharedPIDConfig = true;
        msgFromBytes(&lastReceivedHB, data, sizeof(receiver_heartbeat_t));
        return;
    }
}

void SustainConnectionAction::onCriticalMessage(char *data) {
    FDOS_LOG.print("Error received : ");
    FDOS_LOG.println(data);
}

bool SustainConnectionAction::shouldRequestResponse() {
    TIME_INT_t time = microsSinceEpoch();
    if (time - lastContactSentTime > TRANSMITTER_HB_MIN_MICROS) {
        lastContactSentTime = time;
        return true;
    }
    return false;
}

uint8_t SustainConnectionAction::onSendReady(uint8_t *data, bool &responseExpected) {
    if (connected) {
        if (directESC.commandDurationSeconds > 0) {
            responseExpected = false;
            msgToBytes(&directESC, data, sizeof(directESC));
            analogWrite(LED_R_PIN, 10);
            analogWrite(LED_B_PIN, 10);
            analogWrite(LED_G_PIN, 0);
            directESC.commandDurationSeconds = 0;
            return sizeof(directESC);
        } else if (sharedPIDConfig == false) {
            responseExpected = true;
            flight_config_t config;
            config.yaw_KP = field_PID_yaw_kp.getValue().f;
            config.yaw_KI = field_PID_yaw_ki.getValue().f;
            config.yaw_KD = field_PID_yaw_kd.getValue().f;
            config.yaw_MAX_I = field_PID_yaw_max_i.getValue().i32;

            config.roll_KP = field_PID_roll_kp.getValue().f;
            config.roll_KI = field_PID_roll_ki.getValue().f;
            config.roll_KD = field_PID_roll_kd.getValue().f;
            config.roll_MAX_I = field_PID_roll_max_i.getValue().i32;

            config.pitch_KP = field_PID_pitch_kp.getValue().f;
            config.pitch_KI = field_PID_pitch_ki.getValue().f;
            config.pitch_KD = field_PID_pitch_kd.getValue().f;
            config.pitch_MAX_I = field_PID_pitch_max_i.getValue().i32;

            RADIOTASK.mute(TRANSMITTER_HB_ECHO_DELAY_MILLIS);
            msgToBytes(&config, data, sizeof(config));

            analogWrite(LED_R_PIN, 1);
            analogWrite(LED_B_PIN, 10);
            analogWrite(LED_G_PIN, 2);

            return sizeof(config);
        } else {
            responseExpected = true;
            transmitter_heartbeat_t hb(motorsEngaged, directPitch, directYaw, directRoll, telemEnabled);
            FDOS_LOG.printf("Direct S:%i P:%i Y:%i R:%i\n", hb.PIDMode, hb.isDirectPitch(), hb.isDirectRoll(), hb.isDirectYaw());
            FDOS_LOG.printf("Direct P:%i Y:%i R:%i\n", directPitch, directRoll, directYaw);
            RADIOTASK.mute(TRANSMITTER_HB_ECHO_DELAY_MILLIS);
            msgToBytes(&hb, data, sizeof(transmitter_heartbeat_t));
            analogWrite(LED_R_PIN, 2);
            analogWrite(LED_B_PIN, 2);
            analogWrite(LED_G_PIN, 5);
            return sizeof(transmitter_heartbeat_t);
        }
    } else {
        FDOS_LOG.println("Sending disconnect message");
        data[0] = TRANSMITTER_DISCONNECT;
        data[1] = TRANSMITTER_ID;
        responseExpected = false;
        return 3;
    }
}

TIME_INT_t SustainConnectionAction::getLastReceivedHB(receiver_heartbeat_t &hb) {
    hb = lastReceivedHB;
    return lastContactReceivedTime;
}

void SustainConnectionAction::disconnect() {
    if (!connected)
        return;

    tone(SND_OUTPUT_PIN, 500);
    analogWrite(LED_R_PIN, 100);
    analogWrite(LED_B_PIN, 0);
    analogWrite(LED_G_PIN, 0);
    delay(300);
    tone(SND_OUTPUT_PIN, 0);
    analogWrite(LED_R_PIN, 2);
    analogWrite(LED_B_PIN, 2);
    analogWrite(LED_G_PIN, 2);

    setEngineEngaged(false);

    FDOS_LOG.println("SustainConnection disconnect called.");
    connected = false;
    requestSend();
}

void SustainConnectionAction::setEngineEngaged(bool engaged) {
    if (motorsEngaged == engaged)
        return;
    motorsEngaged = engaged;
    if (engaged) {
        RADIOTASK.addAction((RadioAction *)&transmitCommandAction);
    } else {
        RADIOTASK.removeAction((RadioAction *)&transmitCommandAction);
    }
    requestSend();
}

void SustainConnectionAction::setESC(uint8_t runtimeSeconds, uint8_t *escVals) {
    directESC.commandDurationSeconds = runtimeSeconds;
    directESC.escVals[0] = escVals[0];
    directESC.escVals[1] = escVals[1];
    directESC.escVals[2] = escVals[2];
    directESC.escVals[3] = escVals[3];
    requestSend();
}

void SustainConnectionAction::setTelem(bool telemetryEnabled) {
    telemEnabled = telemetryEnabled;
    requestSend();
}

void SustainConnectionAction::setPIDConfig() {
    sharedPIDConfig = false; // returning PID sent state to false to ensure it is resent and verified
    requestSend();
}

void flightInputListener() { transmitCommandAction.changed(); }

void TransmitCommandAction::onStart() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(MAX_TRANSMIT_PER_SECOND, FrequencyUnitEnum::per_second));

    CONTROLS.joy1H.subscribe(flightInputListener);
    CONTROLS.joy1V.subscribe(flightInputListener);
    CONTROLS.joy2H.subscribe(flightInputListener);
    CONTROLS.joy2V.subscribe(flightInputListener);
}

void TransmitCommandAction::onStop() {
    if (cancel != NULL)
        cancel->cancel();
    cancel = NULL;

    CONTROLS.joy1H.unsubscribe(flightInputListener);
    CONTROLS.joy1V.unsubscribe(flightInputListener);
    CONTROLS.joy2H.unsubscribe(flightInputListener);
    CONTROLS.joy2V.unsubscribe(flightInputListener);

    FDOS_LOG.println("Transmit Action stopped.");
}

void TransmitCommandAction::run(TIME_INT_t time) {
    if (inputChanged) {
        requestSend();
        inputChanged = false;
        analogWrite(LED_R_PIN, 200);
        analogWrite(LED_B_PIN, 200);
        analogWrite(LED_G_PIN, 2);
    } else {
        analogWrite(LED_R_PIN, 2);
        analogWrite(LED_B_PIN, 8);
        analogWrite(LED_G_PIN, 2);
    }
}

void TransmitCommandAction::onReceive(uint8_t length, uint8_t *data, bool responseExpected) {}

uint8_t TransmitCommandAction::onSendReady(uint8_t *data, bool &responseExpected) {
    responseExpected = sustainConnectionAction.shouldRequestResponse();
    if (responseExpected) {
        tone(SND_OUTPUT_PIN, 600, 50);
        RADIOTASK.mute(TRANSMITTER_HB_ECHO_DELAY_MILLIS);
    }

    flight_input_t flightInput;
    flightInput.throttleInput = CONTROLS.joy1V.getUnsignedValue();
    flightInput.slideH = CONTROLS.joy1H.getSignedValue();
    flightInput.joyV = CONTROLS.joy2V.getSignedValue();
    flightInput.joyH = CONTROLS.joy2H.getSignedValue();
    msgToBytes(&flightInput, data, sizeof(flight_input_t));
    return sizeof(flight_input_t);
}

void TelemetryAction::onStart() {}

void TelemetryAction::onStop() {}

void TelemetryAction::requestTelemetry(pid_request_telemetry_t telemRequest) {
    nextRequest = telemRequest;
    requestSend();
}

uint8_t TelemetryAction::onSendReady(uint8_t *data, bool &responseExpected) {
    responseExpected = true;
    msgToBytes(&nextRequest, data, sizeof(pid_request_telemetry_t));
    return sizeof(pid_request_telemetry_t);
}

void TelemetryAction::onReceive(uint8_t length, uint8_t *data, bool responseExpected) {
    if (data[0] == PID_TELEMETRY_RESPONSE) {
        if (receiveListener != NULL) {
            pid_response_telemetry_t response;
            msgFromBytes(&response, data, sizeof(pid_response_telemetry_t));
            FDOS_LOG.printf("Received Telemetry starting at %i elements from a total of %i (len: %i)\n", response.sampleStartIndex, response.totalTelemCount,
                            response.totalTelemCount - response.sampleStartIndex);
            receiveListener(response);
        } else {
            FDOS_LOG.println("Received telemetetry but no listener was set!");
        }
    }
}
