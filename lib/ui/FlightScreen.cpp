#include "FlightScreen.h"
#include "NavScreen.h"
#include <ControllerRadio.h>

FlightScreen FLIGHT_SCREEN;
FlightConfigScreen FLIGHT_CONFIG_SCREEN;
FlightTelemScreen FLIGHT_TELEM_SCREEN;

void drawMotorSpeeds(uint16_t topX, uint16_t topY, uint8_t speeds[4]) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawRFrame(topX, topY, 83, 36, 5);
    UI.getDisplay()->drawBox(topX - 9, topY, 13, 36);
    UI.getDisplay()->setFont(u8g2_font_t0_14_te);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->setFontDirection(1);
    UI.getDisplay()->setCursor(topX + 2, topY + 8);
    UI.getDisplay()->print("ESC");
    UI.getDisplay()->setFontDirection(0);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->setCursor(topX + 8, topY + 5);

    UI.getDisplay()->printf("%3i  %3i", speeds[0], speeds[1]);
    UI.getDisplay()->setCursor(topX + 8, topY + 19);
    UI.getDisplay()->printf("%3i  %3i", speeds[2], speeds[3]);
}

void drawOrientations(uint16_t topX, uint16_t topY, uint16_t current[3], uint16_t target[3]) {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->drawRFrame(topX, topY, 115, 50, 5);
    UI.getDisplay()->drawBox(topX - 9, topY, 13, 50);
    UI.getDisplay()->setFont(u8g2_font_t0_14_te);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->setFontDirection(1);
    UI.getDisplay()->setCursor(topX + 2, topY + 8);
    UI.getDisplay()->print("Y|P|R");
    UI.getDisplay()->setFontDirection(0);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->setDrawColor(0);

    UI.getDisplay()->setCursor(topX + 8, topY + 5);
    UI.getDisplay()->printf("%5.1f->%5.1f", convertHeading(current[0]), convertHeading(target[0]));
    UI.getDisplay()->setCursor(topX + 8, topY + 19);
    UI.getDisplay()->printf("%5.1f->%5.1f", convertHeading(current[1]), convertHeading(target[1]));
    UI.getDisplay()->setCursor(topX + 8, topY + 33);
    UI.getDisplay()->printf("%5.1f->%5.1f", convertHeading(current[2]), convertHeading(target[2]));
}

void flightExitButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        FLIGHT_SCREEN.exitEvent();
    }
}

void radioActivateListener() {
    if (CONTROLS.button1.isPressed()) {
        FLIGHT_SCREEN.toggleRadio();
    }
}

void motorEngageListener() {
    if (CONTROLS.button2.isPressed()) {
        FLIGHT_SCREEN.toggleEngage(); // This function checks if throttle is zerod
    }
}

void toggleDirRoll() {
    if (CONTROLS.button3.isPressed()) {
        sustainConnectionAction.setDirectRoll(!sustainConnectionAction.getDirectRoll());
    }
}

void toggleDirPitch() {
    if (CONTROLS.button4.isPressed()) {
        sustainConnectionAction.setDirectPitch(!sustainConnectionAction.getDirectPitch());
    }
}

void toggleDirYaw() {
    if (CONTROLS.button5.isPressed()) {
        sustainConnectionAction.setDirectYaw(!sustainConnectionAction.getDirectYaw());
    }
}

void configEnableButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        FDOS_LOG.println("Config Enable Listener Clicked");
        FLIGHT_SCREEN.startConfig();
    }
}

void telemEnableButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        FDOS_LOG.println("telem Button Listener Clicked");
        FLIGHT_SCREEN.toggleTelem();
    }
}

FlightConfigScreen::FlightConfigScreen()
    : PID_FIELDS{
          &field_PID_yaw_kp,   &field_PID_yaw_ki,      &field_PID_yaw_kd,  &field_PID_yaw_max_i, &field_PID_pitch_kp, &field_PID_pitch_ki,
          &field_PID_pitch_kd, &field_PID_pitch_max_i, &field_PID_roll_kp, &field_PID_roll_ki,   &field_PID_roll_kd,  &field_PID_roll_max_i,
      } {}

void FlightScreen::startConfig() {
    switch (state) {
    case CONNECTED:
        state = CONFIG;
        FDOS_LOG.println("startConfig called in ConfigState, setting screen!");
        ROOT_UI.setScreen((Screen *)&FLIGHT_CONFIG_SCREEN);
        break;
    default:
        FDOS_LOG.print("startConfig called but in wrong state");
        FDOS_LOG.println(state);
        break;
    }
}

void FlightScreen::toggleEngage() {
    if (state == ENGAGED) {
        state = CONNECTED;
        sustainConnectionAction.setEngineEngaged(false);
    } else if (state == CONNECTED && CONTROLS.joy1V.getUnsignedValue() == 0) {
        state = ENGAGED;
        sustainConnectionAction.setEngineEngaged(true);
        sustainConnectionAction.setTelem(false);
    }
}

void FlightScreen::exitEvent() {
    if (state == OFF || state == PAUSED)
        ROOT_UI.setScreen((Screen *)&NAV_SCREEN);
}

void FlightScreen::start() {
    FDOS_LOG.println("FlightScreen start");
    link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(250, FrequencyUnitEnum::milli));

    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button1.subscribe(radioActivateListener);
    CONTROLS.button2.subscribe(motorEngageListener);

    CONTROLS.wheelBtn.subscribe(configEnableButtonListener);
    CONTROLS.wheelBtn.subscribe(flightExitButtonListener);
    CONTROLS.wheelBtn.subscribe(telemEnableButtonListener);

    CONTROLS.button3.subscribe(toggleDirRoll);
    CONTROLS.button4.subscribe(toggleDirPitch);
    CONTROLS.button5.subscribe(toggleDirYaw);

    CONTROLS.joy1H.setHalfRange(127);
    CONTROLS.joy1V.setHalfRange(127);
    CONTROLS.joy2H.setHalfRange(127);
    CONTROLS.joy2V.setHalfRange(127);

    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 215);
}

void FlightScreen::stop() {
    FDOS_LOG.println("FlightScreen stop");
    if (state != CONFIG) {
        RADIOTASK.removeAllActions();
        state = OFF;
    }

    CONTROLS.wheelBtn.unsubscribe(configEnableButtonListener);
    CONTROLS.wheelBtn.unsubscribe(flightExitButtonListener);
    CONTROLS.wheelBtn.unsubscribe(telemEnableButtonListener);

    CONTROLS.button1.unsubscribe(radioActivateListener);
    CONTROLS.button2.unsubscribe(motorEngageListener);
    CONTROLS.button3.unsubscribe(toggleDirRoll);
    CONTROLS.button4.unsubscribe(toggleDirPitch);
    CONTROLS.button5.unsubscribe(toggleDirYaw);

    link->cancel();
}

void FlightScreen::run(TIME_INT_t time) {
    switch (state) {
    case OFF:
    case PAUSED:
        analogWrite(LED_R_PIN, 3);
        analogWrite(LED_G_PIN, 4);
        analogWrite(LED_R_PIN, 3);

        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 25, 400, 215);
        UI.getDisplay()->setDrawColor(0);

        UI.getDisplay()->setCursor(5, 26);
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        UI.getDisplay()->print("Lora Channel: ");
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->print(field_radio_Freq.getValue().f);
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        UI.getDisplay()->print(" Transmitter ID: ");
        UI.getDisplay()->setFont(u8g2_font_helvB14_tr);
        UI.getDisplay()->print(TRANSMITTER_ID);
        drawNavMenu(EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, "Exit", EMPTY_TITLE, "Activate");
        UI.requestDraw();
        break;
    case SEARCHING: {
        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 50, 400, 215);
        UI.getDisplay()->setDrawColor(0);
        uint8_t step = (time / 1000000) % 5;
        analogWrite(LED_R_PIN, 1);
        analogWrite(LED_B_PIN, 10 * (step % 2));
        analogWrite(LED_G_PIN, 1);
        switch (radioFindReceiverAction.state) {
        case FindReceiverAction::PINGING:
            switch (step) {
            case 0:
                UI.getDisplay()->drawTriangle(20, 50, 60, 50, 40, 70);
                break;
            case 1:
                UI.getDisplay()->drawTriangle(60, 50, 60, 90, 40, 70);
                break;
            case 2:
                UI.getDisplay()->drawTriangle(20, 90, 60, 90, 40, 70);
                break;
            case 3:
                UI.getDisplay()->drawTriangle(20, 50, 20, 90, 40, 70);
                break;
            case 4:
                UI.getDisplay()->drawBox(20, 50, 40, 40);
                break;
            }
            drawNavMenu(EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, "Stop", false, false, false, false, true);
            UI.requestDraw();
            break;
        case FindReceiverAction::CONNECTED:
            state = CONNECTED;
        default:
            return;
        }
        break;
    }
    case CONNECTED:

        if (!sustainConnectionAction.isConnected()) {
            FDOS_LOG.println("Sustained connection dropped, freeing radio");
            toggleRadio();
            return;
        }
        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 25, 400, 215);
        UI.getDisplay()->setDrawColor(0);

        UI.getDisplay()->setCursor(50, 160);
        UI.getDisplay()->setFont(u8g2_font_helvR14_tr);
        UI.getDisplay()->print(CONTROLS.joy1V.getUnsignedValue() == 0 ? "Ready!" : "Zero Throttle before engaging");

        drawInputs();
        drawReceiverStats();

        drawNavMenu("Dir Yaw", "Dir Pch", "Dir Roll", "Config", "Engage", "Disconnect", sustainConnectionAction.getDirectYaw(),
                    sustainConnectionAction.getDirectPitch(), sustainConnectionAction.getDirectRoll(), false, true);
        CONTROLS.button2.setLEDValue(CONTROLS.joy1V.getUnsignedValue() == 0 ? 255 : 0);
        CONTROLS.button1.setLEDValue(200);
        CONTROLS.button3.setLEDValue(sustainConnectionAction.getDirectRoll() * 255);
        CONTROLS.button4.setLEDValue(sustainConnectionAction.getDirectPitch() * 255);
        CONTROLS.button5.setLEDValue(sustainConnectionAction.getDirectYaw() * 255);
        UI.requestDraw();
        break;
    case ENGAGED: {
        if (!sustainConnectionAction.isConnected()) {
            FDOS_LOG.println("Sustained connection dropped, freeing radio");
            toggleRadio();
            return;
        }

        UI.getDisplay()->setDrawColor(1);
        UI.getDisplay()->drawBox(0, 25, 400, 215);
        UI.getDisplay()->setDrawColor(0);
        drawInputs();
        drawReceiverStats();

        drawNavMenu("Dir Yaw", "Dir Pch", "Dir Roll", telemCaptureEnabled ? "Stop" : "Rec", "Disengage", "Disconnect", sustainConnectionAction.getDirectYaw(),
                    sustainConnectionAction.getDirectPitch(), sustainConnectionAction.getDirectRoll(), true, true);

        uint8_t pmwPhase = abs((microsSinceEpoch() / 10000) % 255 - 128) * 2;

        CONTROLS.button3.setLEDValue(sustainConnectionAction.getDirectRoll() * 255);
        CONTROLS.button4.setLEDValue(sustainConnectionAction.getDirectPitch() * 255);
        CONTROLS.button5.setLEDValue(sustainConnectionAction.getDirectYaw() * 255);

        CONTROLS.button2.setLEDValue(pmwPhase);
        CONTROLS.button1.setLEDValue(255);

        UI.requestDraw();
        break;
    }
    case CONFIG:
        state = CONNECTED;
        break;
    }
}

void FlightScreen::drawInputs() {
    drawXYInput(0, 25, 100, CONTROLS.joy2H.getUnsignedValue(), CONTROLS.joy2V.getUnsignedValue(), 255, true);
    drawHInput(0, 127, 100, 10, CONTROLS.joy1H.getUnsignedValue(), 255, true);
    drawVInput(102, 25, 100, 10, CONTROLS.joy1V.getUnsignedValue(), 255, true, false);
}

void FlightScreen::drawReceiverStats() {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setCursor(140, 28);
    receiver_heartbeat_t hb;
    sustainConnectionAction.getLastReceivedHB(hb);
    UI.getDisplay()->setFont(u8g2_font_10x20_tr);
    UI.getDisplay()->printf("Rx:%3i%%  Tx:%3i%%", hb.snr, (int16_t)RADIO.getSNR() * 10);
    UI.getDisplay()->setCursor(140, 43);
    UI.getDisplay()->printf("Vlt:%2.2f Cur:%2.2f(%i)", hb.getBatteryVolts(), hb.getCurrentAmps(), hb.cur);

    drawMotorSpeeds(145, 64, hb.speeds);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->setCursor(142, 101);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->printf("P.MB:%5.1f", convertPressure(hb.pressure));

    drawOrientations(242, 64, hb.headings, hb.targetHeadings);
}

void FlightScreen::toggleTelem() {
    if (telemCaptureEnabled) {
        FDOS_LOG.println("Telem off");
        telemCaptureEnabled = false;
        sustainConnectionAction.setTelem(telemCaptureEnabled);
    } else {
        if (state == ENGAGED) {
            telemCaptureEnabled = true;
            sustainConnectionAction.setTelem(telemCaptureEnabled);
        }
    }
}

void FlightScreen::toggleRadio() {
    if (state == PAUSED) {
        FDOS_LOG.println("Unpausing");
        state = SEARCHING;
        RADIOTASK.addAction((RadioAction *)&radioFindReceiverAction);
        return;
    }
    if (state == OFF) {
        UI.getDisplay()->setFont(u8g2_font_t0_14b_te);
        UI.getDisplay()->setCursor(20, 50);
        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->print("Starting Radio ");
        UI.requestDraw(true);

        delay(250);

        float linkBW = 500;
        switch (field_radio_Linkbw.getValue().u8) {
        case 0:
            linkBW = 100;
            break;
        case 1:
            linkBW = 250;
            break;
        case 2:
            linkBW = 500;
            break;
        }
        int rstate = RADIO.begin(field_radio_Freq.getValue().f, linkBW, field_radio_SpreadingFactor.getValue().u8, field_radio_CodingRate.getValue().u8);
        UI.getDisplay()->setDrawColor(0);
        if (rstate == RADIOLIB_ERR_NONE) {
            UI.getDisplay()->println("...");
            UI.requestDraw(true);
        } else {
            UI.getDisplay()->print("SX1276 Init failed. Code:");
            UI.getDisplay()->println(rstate);
            UI.requestDraw(true);
            digitalWrite(LED_R_PIN, 1);
            delay(5000);

            return;
        }

        if (RADIO.setOutputPower(field_radio_Power.getValue().u8) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
            UI.getDisplay()->println("Invalid Power Setting.");
            UI.requestDraw(true);
            delay(5000);
            digitalWrite(LED_R_PIN, 1);

            return;
        }
        UI.getDisplay()->setDrawColor(0);
        UI.getDisplay()->println("Started!");
        UI.requestDraw(true);
        delay(500);
        state = SEARCHING;
        RADIOTASK.addAction((RadioAction *)&radioFindReceiverAction);
        return;
    }
    FDOS_LOG.println("Pausing");
    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button2.setLEDValue(0);
    state = PAUSED;
    RADIOTASK.removeAllActions();
    return;
}

/**
 * FLIGHT CONFIG SCREEN
 **/

void configExitButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_SCREEN);
    }
}

void configNavLeftRightButtonListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.LEFT)
        FLIGHT_CONFIG_SCREEN.moveSelection(-1);
    else if (CONTROLS.arrows.getState() == CONTROLS.RIGHT)
        FLIGHT_CONFIG_SCREEN.moveSelection(1);
}

void configEditWheelListener() {
    if (CONTROLS.wheel.hasChanged()) {
        FLIGHT_CONFIG_SCREEN.changeValue(CONTROLS.wheel.getDelta());
    }
}

void configEditArrowListener() {
    if (CONTROLS.arrows.getState() == CONTROLS.RIGHT) {
        FLIGHT_CONFIG_SCREEN.changeValue(50);
    } else if (CONTROLS.arrows.getState() == CONTROLS.LEFT) {
        FLIGHT_CONFIG_SCREEN.changeValue(-50);
    }
    if (CONTROLS.arrows.getState() == CONTROLS.DOWN) {
        FLIGHT_CONFIG_SCREEN.changeValue(500);
    } else if (CONTROLS.arrows.getState() == CONTROLS.UP) {
        FLIGHT_CONFIG_SCREEN.changeValue(-500);
    }
}

void configEditButtonListener() {
    if (CONTROLS.button2.isPressed())
        FLIGHT_CONFIG_SCREEN.startChange();
}

void configResetButtonListener() {
    if (CONTROLS.button1.isPressed())
        FLIGHT_CONFIG_SCREEN.resetValue();
}

void configSaveButtonListener() {
    if (CONTROLS.button4.isPressed())
        FLIGHT_CONFIG_SCREEN.saveChange();
}
void configCancelButtonListener() {
    if (CONTROLS.button3.isPressed())
        FLIGHT_CONFIG_SCREEN.cancelChange();
}

void configESCButtonListener() {
    if (CONTROLS.button5.isPressed())
        FLIGHT_CONFIG_SCREEN.sendESC();
}

void configPIDButtonListener() {
    if (CONTROLS.button4.isPressed())
        FLIGHT_CONFIG_SCREEN.sendPIDSettings();
}

void openTelemetryButtonListener() {
    if (CONTROLS.button3.isPressed())
        ROOT_UI.setScreen((Screen *)&FLIGHT_TELEM_SCREEN);
}

void FlightConfigScreen::moveSelection(int8_t move) {
    partialValueChange = 0;
    selection += move;
    if (selection > maxSelection) {
        if (move > 0)
            selection = 0;
        else
            selection = maxSelection;
    }
}

void FlightConfigScreen::changeValue(int16_t change) {

    if (!editing)
        return;

    partialValueChange += change % 2;

    if (partialValueChange % 2 == 0) {
        change += partialValueChange;
        partialValueChange = 0;
    }

    change = change / 2;

    if (change == 0)
        return;

    switch (selection) {
    case 0:
        runtime += change;
        if (runtime > 60)
            runtime = 0;
        if (runtime < 0)
            runtime = 60;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        escVals[selection - 1] += change;
        break;
    default:
        if (change != 0) {
            PID_FIELDS[selection - 5]->modify(change);
        }
        break;
    }
}

void FlightConfigScreen::startChange() {
    if (editing)
        return;
    editing = true;
    exitSelectMode();
    setupEditMode();
}

void FlightConfigScreen::saveChange() {
    if (!editing)
        return;
    editing = false;

    if (selection > 4)
        PID_FIELDS[selection - 5]->save();

    exitEditMode();
    setupSelectMode();
}

void FlightConfigScreen::cancelChange() {
    if (selection == 0)
        runtime = 5;
    else if (selection > 4)
        PID_FIELDS[selection - 5]->cancel();
    else
        escVals[selection - 1] = 0;
    exitEditMode();
    setupSelectMode();
}

void FlightConfigScreen::resetValue() {

    if (selection == 0)
        runtime = 5;
    else if (selection > 4)
        PID_FIELDS[selection - 5]->overwriteWithDefault();
    else
        escVals[selection - 1] = 0;

    if (editing) {
        editing = false;
        exitEditMode();
        setupSelectMode();
    }
}

void FlightConfigScreen::sendESC() { sustainConnectionAction.setESC(runtime, escVals); }

void FlightConfigScreen::sendPIDSettings() { sustainConnectionAction.setPIDConfig(); }

void FlightConfigScreen::exitEditMode() {
    CONTROLS.button1.unsubscribe(configResetButtonListener);
    CONTROLS.button3.unsubscribe(configCancelButtonListener);
    CONTROLS.button4.unsubscribe(configSaveButtonListener);
    CONTROLS.arrows.unsubscribe(configEditArrowListener);
    CONTROLS.wheel.unsubscribe(configEditWheelListener);
}

void FlightConfigScreen::exitSelectMode() {
    CONTROLS.wheelBtn.unsubscribe(configExitButtonListener);
    CONTROLS.button1.unsubscribe(configResetButtonListener);
    CONTROLS.button2.unsubscribe(configEditButtonListener);
    CONTROLS.button3.unsubscribe(openTelemetryButtonListener);
    CONTROLS.button4.unsubscribe(configPIDButtonListener);
    CONTROLS.button5.unsubscribe(configESCButtonListener);
    CONTROLS.arrows.unsubscribe(configNavLeftRightButtonListener);
}

void FlightConfigScreen::setupEditMode() {
    CONTROLS.button1.subscribe(configResetButtonListener);
    CONTROLS.button3.subscribe(configCancelButtonListener);
    CONTROLS.button4.subscribe(configSaveButtonListener);
    CONTROLS.arrows.subscribe(configEditArrowListener);
    CONTROLS.wheel.subscribe(configEditWheelListener);
    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button2.setLEDValue(0);
    CONTROLS.button3.setLEDValue(255);
    CONTROLS.button4.setLEDValue(255);
    CONTROLS.button5.setLEDValue(0);

    drawNavMenu(EMPTY_TITLE, "Save", "Cancel", EMPTY_TITLE, EMPTY_TITLE, "Reset", false, true, true, false, true);
}

void FlightConfigScreen::setupSelectMode() {
    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button2.setLEDValue(255);
    CONTROLS.button3.setLEDValue(255);
    CONTROLS.button4.setLEDValue(255);
    CONTROLS.button5.setLEDValue(255);

    CONTROLS.wheelBtn.subscribe(configExitButtonListener);
    CONTROLS.button1.subscribe(configResetButtonListener);
    CONTROLS.button2.subscribe(configEditButtonListener);
    CONTROLS.button3.subscribe(openTelemetryButtonListener);

    CONTROLS.button4.subscribe(configPIDButtonListener);
    CONTROLS.button5.subscribe(configESCButtonListener);
    CONTROLS.arrows.subscribe(configNavLeftRightButtonListener);
    drawNavMenu("Set ESC", "Set PID", "Telemetry", "Exit", "Edit", "Reset", true, true, true, true, true);
}

void FlightConfigScreen::start() {
    FDOS_LOG.println("FlightConfigScreen start");
    link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(250, FrequencyUnitEnum::milli));

    setupSelectMode();

    editing = false;
}

void FlightConfigScreen::stop() {
    FDOS_LOG.println("FlightConfigScreen stop");
    link->cancel();

    if (editing) {
        exitEditMode();
    } else {
        exitSelectMode();
    }
}

void FlightConfigScreen::drawESC() {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setCursor(20, 50);
    UI.getDisplay()->setFont(u8g2_font_t0_16_te);
    UI.getDisplay()->printf("For: %3i sec", runtime);
    drawMotorSpeeds(45, 69, escVals);
    UI.getDisplay()->setDrawColor(0);
    switch (selection) {
    case 0:
        UI.getDisplay()->drawFrame(57, 48, 28, 17);
        break;
    case 1:
        UI.getDisplay()->drawFrame(50, 71, 30, 17);
        break;
    case 2:
        UI.getDisplay()->drawFrame(92, 71, 30, 17);
        break;
    case 3:
        UI.getDisplay()->drawFrame(50, 86, 30, 17);
        break;
    case 4:
        UI.getDisplay()->drawFrame(92, 86, 30, 17);
        break;
    }
}

void FlightConfigScreen::drawPID() {
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setCursor(178, 53);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->print("KP     KI     KD   MaxI");

    UI.getDisplay()->drawRFrame(148, 69, 220, 53, 5);
    UI.getDisplay()->drawBox(148, 69, 13, 53);
    UI.getDisplay()->setFont(u8g2_font_t0_14_te);
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->setFontDirection(1);
    UI.getDisplay()->setCursor(159, 78);
    UI.getDisplay()->print("Y|P|R");
    UI.getDisplay()->setFontDirection(0);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->setDrawColor(0);

    uint16_t x = 168, y = 71;

    for (uint8_t i = 0; i < PID_FIELD_COUNT; i++) {

        UI.getDisplay()->setDrawColor(0);
        if (i == selection - 5)
            UI.getDisplay()->drawFrame(x - 5, y - 1, 50, 15);
        UI.getDisplay()->setCursor(x, y);

        UI.getDisplay()->print(PID_FIELDS[i]->getText());
        if (i % 4 == 3) {
            y += 18;
            x = 168;
        } else {
            x += 53;
        }
    }
}

void FlightConfigScreen::run(TIME_INT_t time) {
    if (!sustainConnectionAction.isConnected()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_SCREEN);
        return;
    }
    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 155);
    UI.getDisplay()->setDrawColor(0);

    UI.getDisplay()->setCursor(10, 28);
    receiver_heartbeat_t hb;
    sustainConnectionAction.getLastReceivedHB(hb);
    UI.getDisplay()->setFont(u8g2_font_t0_16b_te);
    UI.getDisplay()->printf("Rx:%3i%% Tx:%3i%%", hb.snr, (int16_t)RADIO.getSNR() * 10);
    UI.getDisplay()->setCursor(160, 28);
    UI.getDisplay()->printf("Vlt:%2.2f Cur:%2.2f(%i)", hb.getBatteryVolts(), hb.getCurrentAmps(), hb.cur);

    drawESC();
    drawPID();

    UI.requestDraw();
}

void telemExitButtonListener() {
    if (CONTROLS.button1.isPressed()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_CONFIG_SCREEN);
    }
}

void telemSerialButtonListener() {
    if (CONTROLS.button2.isPressed()) {
        FLIGHT_TELEM_SCREEN.dumpToSerial();
    }
}

void telemWheelListener() {
    if (CONTROLS.wheel.hasChanged()) {
        FLIGHT_TELEM_SCREEN.scroll(CONTROLS.wheel.getDelta());
    }
}

void telemArrowListener() {
    switch (CONTROLS.arrows.getState()) {
    case CONTROLS.UP:
        FLIGHT_TELEM_SCREEN.toTop();
        break;
    case CONTROLS.DOWN:
        FLIGHT_TELEM_SCREEN.toBottom();
        break;
    case CONTROLS.LEFT:
        FLIGHT_TELEM_SCREEN.scroll(-20);
        break;
    case CONTROLS.RIGHT:
        FLIGHT_TELEM_SCREEN.scroll(20);
        break;
    }
}

void telemTabButtonListener() {
    if (CONTROLS.button3.isPressed()) {
        FLIGHT_TELEM_SCREEN.showRoll();
    } else if (CONTROLS.button4.isPressed()) {
        FLIGHT_TELEM_SCREEN.showPitch();
    } else if (CONTROLS.button5.isPressed()) {
        FLIGHT_TELEM_SCREEN.showYaw();
    }
}

void FlightTelemScreen::scroll(int16_t change) {
    partialValueChange += change % 2;

    if (partialValueChange % 2 == 0) {
        change += partialValueChange;
        partialValueChange = 0;
    }

    change = change / 2;

    if (change == 0)
        return;

    uint16_t oldSelection = selection;

    selection += change * 15;

    if (!requestTelemData()){
        selection = oldSelection;
    }
}

void onTelemReceive(pid_response_telemetry_t telem) { FLIGHT_TELEM_SCREEN.onTelemReceived(telem); }

void FlightTelemScreen::start() {
    FDOS_LOG.println("FlightTelemScreen start");
    link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(250, FrequencyUnitEnum::milli));

    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button1.subscribe(telemExitButtonListener);
    CONTROLS.button2.setLEDValue(255);
    CONTROLS.button2.subscribe(telemSerialButtonListener);

    CONTROLS.button3.subscribe(telemTabButtonListener);
    CONTROLS.button4.subscribe(telemTabButtonListener);
    CONTROLS.button5.subscribe(telemTabButtonListener);

    CONTROLS.button3.setLEDValue(0);
    CONTROLS.button4.setLEDValue(0);
    CONTROLS.button5.setLEDValue(0);

    CONTROLS.arrows.subscribe(telemArrowListener);
    CONTROLS.wheel.subscribe(telemWheelListener);

    telemetryAction.setReceiveListener(onTelemReceive);
    RADIOTASK.addAction((RadioAction *)&telemetryAction);

    telemResponseData.sampleStartIndex = 0;
    telemResponseData.totalTelemCount = 0;
    selection = 0;
    stateYPR = 0;

    pitchPID = new PID(field_PID_pitch_kp.getValue().f, field_PID_pitch_ki.getValue().f, field_PID_pitch_kd.getValue().f, PID_PITCH_MAXOUT, PID_PITCH_INPERIOD,
                       field_PID_pitch_max_i.getValue().f, 3);
    yawPID = new PID(field_PID_yaw_kp.getValue().f, field_PID_yaw_ki.getValue().f, field_PID_yaw_kd.getValue().f, PID_YAW_MAXOUT, PID_YAW_INPERIOD,
                     field_PID_yaw_max_i.getValue().f, 3);
    rollPID = new PID(field_PID_roll_kp.getValue().f, field_PID_roll_ki.getValue().f, field_PID_roll_kd.getValue().f, PID_ROLL_MAXOUT, PID_ROLL_INPERIOD,
                      field_PID_roll_max_i.getValue().f, 3);

    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 215);

    requestTelemData();
}

void FlightTelemScreen::stop() {
    FDOS_LOG.println("FlightTelemScreen stop");

    RADIOTASK.removeAction((RadioAction *)&telemetryAction);

    link->cancel();

    delete pitchPID;
    delete rollPID;
    delete yawPID;

    CONTROLS.button1.unsubscribe(telemExitButtonListener);
    CONTROLS.button2.unsubscribe(telemSerialButtonListener);

    CONTROLS.button3.unsubscribe(telemTabButtonListener);
    CONTROLS.button4.unsubscribe(telemTabButtonListener);
    CONTROLS.button5.unsubscribe(telemTabButtonListener);

    CONTROLS.arrows.unsubscribe(telemArrowListener);
    CONTROLS.wheel.unsubscribe(telemWheelListener);
}

void FlightTelemScreen::run(TIME_INT_t time) {
    if (!sustainConnectionAction.isConnected()) {
        ROOT_UI.setScreen((Screen *)&FLIGHT_SCREEN);
        return;
    }

    CONTROLS.button3.setLEDValue(255 * (stateYPR == 2));
    CONTROLS.button4.setLEDValue(255 * (stateYPR == 1));
    CONTROLS.button5.setLEDValue(255 * (stateYPR == 0));

    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 215);
    UI.getDisplay()->setDrawColor(0);

    drawTelemScroll(telemResponseData.sampleStartIndex, telemResponseData.totalTelemCount);
    drawTelemData(telemResponseData.samples, telemResponseData.totalTelemCount - telemResponseData.sampleStartIndex, telemResponseData.sampleDurationMicros);

    drawNavMenu("Yaw", "Pitch", "Roll", EMPTY_TITLE, "Serial", "Exit", stateYPR == 0, stateYPR == 1, stateYPR == 2, true, true, true);

    UI.requestDraw();
}

bool FlightTelemScreen::requestTelemData() {
    pid_request_telemetry_t request;
    request.index = selection;
    request.telemDimensionYPR = stateYPR;
    return telemetryAction.requestTelemetry(request);
}

void FlightTelemScreen::drawTelemScroll(uint16_t index, uint16_t total) { drawVInput(18, 30, 140, 12, index, total, true, true); }

void FlightTelemScreen::drawTelemData(pid_state_t *samples, uint8_t size, uint16_t sampleDurationMicros) {
    uint16_t y = 25;
    UI.getDisplay()->setFont(u8g2_font_t0_13_te);
    UI.getDisplay()->setDrawColor(0);
    UI.getDisplay()->setCursor(30, y);
    UI.getDisplay()->print("TARGT:ERROR| OUT=PCONTR+ICONTR+DCONTR| ITOTL, DERIV");
    PID *pid = NULL;
    if (stateYPR == 0)
        pid = yawPID;
    else if (stateYPR == 1)
        pid = pitchPID;
    if (stateYPR == 2)
        pid = rollPID;

    for (uint8_t i = 0; i < min(15, telemResponseData.totalTelemCount - telemResponseData.sampleStartIndex); i++) {
        y += 12;
        UI.getDisplay()->setCursor(30, y);
        UI.getDisplay()->printf("%5i:%5i]%5i=%5i+%5i+%5i|%5i,%5i", samples[i].target, samples[i].observedError,
                                pid->calculateOutput(pid->calculatePContribution(samples[i].observedError),
                                                     pid->calculateIContribution(samples[i].integral),
                                                     pid->calculateDContribution(samples[i].derivative)),
                                pid->calculatePContribution(samples[i].observedError), pid->calculateIContribution(samples[i].integral),
                                pid->calculateDContribution(samples[i].derivative), samples[i].integral, samples[i].derivative);
    }
}

void FlightTelemScreen::onTelemReceived(pid_response_telemetry_t telem) {
    telemResponseData = telem;

    if (pitchPID == NULL || yawPID == NULL || rollPID == NULL) {
        FDOS_LOG.println("ERROR!  Cant print Flight Telemetry, local PID not initiated");
        return;
    }

    if (transmitAll) {
        for (uint8_t i = 0; i < min(15, telemResponseData.totalTelemCount - telemResponseData.sampleStartIndex); i++) {
            FDOS_LOG.print(i + telemResponseData.sampleStartIndex);
            switch (stateYPR) {
            case 0:
                FDOS_LOG.print("Y,");
                telemResponseData.samples[i].printCSV(&FDOS_LOG, *yawPID);
                break;
            case 1:
                FDOS_LOG.print("P,");
                telemResponseData.samples[i].printCSV(&FDOS_LOG, *pitchPID);
                break;
            case 2:
                FDOS_LOG.print("R,");
                telemResponseData.samples[i].printCSV(&FDOS_LOG, *rollPID);
                break;
            }
        }

        if (telem.totalTelemCount - telem.sampleStartIndex <= PID_SAMPLE_SIZE) {
            transmitAll = false;
        } else {
            selection += PID_SAMPLE_SIZE;
            requestTelemData(); // since this is triggered by a telemReceive event, it should be safe to send
        }
    } else {
        FDOS_LOG.printf("Sample #%i of %i.  Sample duration (micros) %i\n", telemResponseData.sampleStartIndex, telemResponseData.totalTelemCount,
                        telemResponseData.sampleDurationMicros);
        for (uint8_t i = 0; i < min(15, telemResponseData.totalTelemCount - telemResponseData.sampleStartIndex); i++) {
            FDOS_LOG.print(i + telemResponseData.sampleStartIndex);
            switch (stateYPR) {
            case 0:
                FDOS_LOG.print("Y,");
                telemResponseData.samples[i].print(&FDOS_LOG, *yawPID);
                break;
            case 1:
                FDOS_LOG.print("P,");
                telemResponseData.samples[i].print(&FDOS_LOG, *pitchPID);
                break;
            case 2:
                FDOS_LOG.print("R,");
                telemResponseData.samples[i].print(&FDOS_LOG, *rollPID);
                break;
            }
        }
    }
}

void FlightTelemScreen::dumpToSerial() {
    transmitAll = true;
    selection = 0;
    requestTelemData();
}

void FlightTelemScreen::toTop() {
    selection = 0;
    requestTelemData();
}

void FlightTelemScreen::toBottom() {
    if (telemResponseData.totalTelemCount > PID_SAMPLE_SIZE)
        selection = telemResponseData.totalTelemCount - PID_SAMPLE_SIZE;
    else
        selection = 0;
    requestTelemData();
}

void FlightTelemScreen::showYaw() {
    stateYPR = 0;
    requestTelemData();
}

void FlightTelemScreen::showPitch() {
    stateYPR = 1;
    requestTelemData();
}

void FlightTelemScreen::showRoll() {
    stateYPR = 2;
    requestTelemData();
}
