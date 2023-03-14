#include "FlightScreen.h"
#include <ControllerRadio.h>

FlightScreen FLIGHT_SCREEN;
FlightConfigScreen FLIGHT_CONFIG_SCREEN;

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

void toggleDirPitch() {
    if (CONTROLS.button3.isPressed()) {
        sustainConnectionAction.setDirectPitch(!sustainConnectionAction.getDirectPitch());
    }
}

void toggleDirRoll() {
    if (CONTROLS.button4.isPressed()) {
        sustainConnectionAction.setDirectRoll(!sustainConnectionAction.getDirectRoll());
    }
}

void toggleDirYaw() {
    if (CONTROLS.button5.isPressed()) {
        sustainConnectionAction.setDirectYaw(!sustainConnectionAction.getDirectYaw());
    }
}

void configEnableButtonListener() {
    if (CONTROLS.wheelBtn.isPressed()) {
        FLIGHT_SCREEN.startConfig();
    }
}

FlightConfigScreen::FlightConfigScreen()
    : PID_FIELDS{
          &field_PID_yaw_kp,  &field_PID_yaw_ki,     &field_PID_yaw_kd,   &field_PID_yaw_max_i, &field_PID_roll_kp,  &field_PID_roll_ki,
          &field_PID_roll_kd, &field_PID_roll_max_i, &field_PID_pitch_kp, &field_PID_pitch_ki,  &field_PID_pitch_kd, &field_PID_pitch_max_i,
      } {}

void FlightScreen::startConfig() {
    switch (state) {
    case CONNECTED:
        state = CONFIG;
        ROOT_UI.setScreen((Screen *)&FLIGHT_CONFIG_SCREEN);
        break;
    default:
        break;
    }
}

void FlightScreen::toggleEngage() {
    if (state == ENGAGED) {
        state = CONNECTED;
        sustainConnectionAction.setEngineEngaged(false);
        CONTROLS.wheelBtn.subscribe(configEnableButtonListener);
    } else if (state == CONNECTED && CONTROLS.joy1V.getUnsignedValue() == 0) {
        state = ENGAGED;
        sustainConnectionAction.setEngineEngaged(true);
        CONTROLS.wheelBtn.unsubscribe(configEnableButtonListener);
    }
}

void FlightScreen::start() {
    link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(250, FrequencyUnitEnum::milli));

    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button1.subscribe(radioActivateListener);
    CONTROLS.button2.subscribe(motorEngageListener);
    if (state == CONFIG) {
        CONTROLS.wheelBtn.subscribe(configEnableButtonListener);
        state = CONNECTED;
    } else {
        CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
        state = OFF;
    }

    CONTROLS.button3.subscribe(toggleDirPitch);
    CONTROLS.button4.subscribe(toggleDirRoll);
    CONTROLS.button5.subscribe(toggleDirYaw);

    CONTROLS.joy1H.setHalfRange(127);
    CONTROLS.joy1V.setHalfRange(127);
    CONTROLS.joy2H.setHalfRange(127);
    CONTROLS.joy2V.setHalfRange(127);

    UI.getDisplay()->setDrawColor(1);
    UI.getDisplay()->drawBox(0, 25, 400, 215);
}

void FlightScreen::stop() {
    if (state != CONFIG)
        RADIOTASK.removeAllActions();

    if (state != CONNECTED)
        CONTROLS.wheelBtn.unsubscribe(configEnableButtonListener);
    else
        CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);

    CONTROLS.button1.unsubscribe(radioActivateListener);
    CONTROLS.button2.unsubscribe(motorEngageListener);
    CONTROLS.button3.unsubscribe(toggleDirPitch);
    CONTROLS.button4.unsubscribe(toggleDirRoll);
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
            drawNavMenu(EMPTY_TITLE, EMPTY_TITLE, EMPTY_TITLE, "Exit", EMPTY_TITLE, "Stop", false, false, false, false, true);
            UI.requestDraw();
            break;
        case FindReceiverAction::CONNECTED:
            state = CONNECTED;
            CONTROLS.wheelBtn.unsubscribe(navEnableButtonListener);
            CONTROLS.wheelBtn.subscribe(configEnableButtonListener);
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

        drawNavMenu("Dir Yaw", "Dir Roll", "Dir Pch", "Config", "Engage", "Disconnect", sustainConnectionAction.getDirectYaw(),
                    sustainConnectionAction.getDirectRoll(), sustainConnectionAction.getDirectPitch(), false, true);
        CONTROLS.button2.setLEDValue(CONTROLS.joy1V.getUnsignedValue() == 0 ? 255 : 0);
        CONTROLS.button1.setLEDValue(200);
        CONTROLS.button3.setLEDValue(sustainConnectionAction.getDirectPitch() * 200);
        CONTROLS.button4.setLEDValue(sustainConnectionAction.getDirectRoll() * 200);
        CONTROLS.button5.setLEDValue(sustainConnectionAction.getDirectYaw() * 200);
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

        drawNavMenu("Dir Yaw", "Dir Roll", "Dir Pch", EMPTY_TITLE, "Disengage", "Disconnect", sustainConnectionAction.getDirectYaw(),
                    sustainConnectionAction.getDirectRoll(), sustainConnectionAction.getDirectPitch(), true, true);

        uint8_t pmwPhase = abs((microsSinceEpoch() / 10000) % 255 - 128) * 2;

        CONTROLS.button3.setLEDValue(sustainConnectionAction.getDirectPitch() ? 255 - pmwPhase : 0);
        CONTROLS.button4.setLEDValue(sustainConnectionAction.getDirectRoll() ? pmwPhase : 0);
        CONTROLS.button5.setLEDValue(sustainConnectionAction.getDirectYaw() ? 255 - pmwPhase : 0);

        CONTROLS.button2.setLEDValue(pmwPhase);
        CONTROLS.button1.setLEDValue(255);

        UI.requestDraw();
        break;
    }
    case CONFIG:
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
    CONTROLS.wheelBtn.subscribe(navEnableButtonListener);
    return;
}

/**
 * FLIGHT CONFIG SCREEN
 **/

void configExitButtonListener() {
    if (CONTROLS.button1.isPressed()) {
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
    if (CONTROLS.button2.isPressed()) {
        FLIGHT_CONFIG_SCREEN.startChange();
    }
}

void configSaveButtonListener() { FLIGHT_CONFIG_SCREEN.saveChange(); }
void configCancelButtonListener() { FLIGHT_CONFIG_SCREEN.cancelChange(); }

void configESCButtonListener() {
    if (CONTROLS.button5.isPressed()) {
        FLIGHT_CONFIG_SCREEN.sendESC();
    }
}

void configPIDButtonListener() {
    if (CONTROLS.button4.isPressed()) {
        FLIGHT_CONFIG_SCREEN.sendPIDSettings();
    }
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

void FlightConfigScreen::changeValue(int8_t change) {

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
    if (editing == true)
        return;
    editing = true;
    exitSelectMode();
    CONTROLS.wheel.subscribe(configEditWheelListener);
    CONTROLS.button1.subscribe(configEditWheelListener);

    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button2.setLEDValue(0);
    CONTROLS.button3.setLEDValue(255);
    CONTROLS.button4.setLEDValue(255);
    CONTROLS.button5.setLEDValue(0);
}

void FlightConfigScreen::saveChange() {
    if (!editing)
        return;
    if (selection > 4)
        PID_FIELDS[selection - 5]->save();
}

void FlightConfigScreen::sendESC() { sustainConnectionAction.setESC(runtime, escVals); }

void FlightConfigScreen::sendPIDSettings() { sustainConnectionAction.setPIDConfig(); }

void FlightConfigScreen::exitEditMode() {
    CONTROLS.button3.unsubscribe(configCancelButtonListener);
    CONTROLS.button4.unsubscribe(configEditArrowListener);
    CONTROLS.arrows.unsubscribe(configSaveButtonListener);

    CONTROLS.wheel.unsubscribe(configEditWheelListener);
}

void FlightConfigScreen::exitSelectMode() {
    CONTROLS.button1.unsubscribe(configExitButtonListener);
    CONTROLS.button2.unsubscribe(configEditButtonListener);
    // CONTROLS.button3.unsubscribe(openTelemetryButtonListener);
    CONTROLS.button4.unsubscribe(configPIDButtonListener);
    CONTROLS.button5.unsubscribe(configESCButtonListener);
    CONTROLS.arrows.unsubscribe(configNavLeftRightButtonListener);
}

void FlightConfigScreen::setupEditMode() {}

void FlightConfigScreen::setupSelectMode() {
    CONTROLS.button1.setLEDValue(255);
    CONTROLS.button2.setLEDValue(255);
    CONTROLS.button1.subscribe(configExitButtonListener);
    CONTROLS.button2.subscribe(configEditButtonListener);
    // CONTROLS.button3.subscribe(openTelemetryButtonListener);
    // CONTROLS.button3.setLEDValue(255);
    CONTROLS.button4.subscribe(configPIDButtonListener);
    CONTROLS.button4.setLEDValue(255);
    CONTROLS.button5.subscribe(configESCButtonListener);
    CONTROLS.button5.setLEDValue(255);
    CONTROLS.arrows.subscribe(configNavLeftRightButtonListener);
}

void FlightConfigScreen::start() {
    link = EXECUTOR.schedule((RunnableTask *)this, EXECUTOR.getTimingPair(250, FrequencyUnitEnum::milli));

    setupSelectMode();

    editing = false;
}

void FlightConfigScreen::stop() {
    link->cancel();
    CONTROLS.arrows.unsubscribe(configNavLeftRightButtonListener);
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
    UI.getDisplay()->print("KP     KI     KD    MaxI");

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
    UI.getDisplay()->drawBox(0, 25, 400, 215);
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

    drawNavMenu("Set ESC", "Set PID", "Telemetry", EMPTY_TITLE, EMPTY_TITLE, "Fly", true, true, true, false, true);

    UI.requestDraw();
}
