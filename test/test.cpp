
#include <Arduino.h>
#include <inttypes.h>

#include "FDOS_LOG.h"
#include "VMExecutor.h"
#include "VMTime.h"

#define POWER_LED_PIN 13

#define BAT_V_SENSE_PIN 33
#define BAT_V_MIN_LVL 540 //(aprox 3.15v)

#define BAT_SENSE_ENABLE_PIN 35
#define POWER_ENABLE_SENSE_PIN 34
#define POWER_PRESS_SENSE_LVL 700

#define JOY_BTN_PIN 30
#define JOY_H_PIN 31
#define JOY_V_PIN 32

#define MIN_MICRO_REST 10

Logger FDOS_LOG(&Serial);
VMExecutor executor;

class BlinkTask : public RunnableTask {
    bool flipper = false;

  public:
    void run(TIME_INT_t time) {
        digitalWrite(13, flipper = !flipper);
        Serial.println(time);
    }
};

// void pollInputs(int intervalMS) {
//     pinMode(JOY_BTN_PIN, INPUT_PULLUP);
//     pinMode(JOY_H_PIN, INPUT);
//     pinMode(JOY_V_PIN, INPUT);

//     uint16_t printCounter = 0;
//     while (pollEnabled) {
//         threads.delay(intervalMS);
//         bool joyButtonPressed = !digitalRead(JOY_BTN_PIN);
//         int16_t hPos = analogRead(JOY_H_PIN) - 512;
//         int16_t vPos = analogRead(JOY_V_PIN) - 512;

//         if (printCounter == 5) {
//             Serial.print("***INPUTS*** Joy Btn:");
//             Serial.print(joyButtonPressed);
//             Serial.print(" Joy H:");
//             Serial.print(hPos);
//             Serial.print(" Joy V:");
//             Serial.println(vPos);
//             printCounter = 0;
//         } else {
//             printCounter++;
//         }

//         digitalWrite(POWER_LED_PIN, joyButtonPressed);
//     }
// }

// void managePower(int intervalMS) {

//     pinMode(BAT_SENSE_ENABLE_PIN, OUTPUT);

//     int16_t sampleCounter = -1; // First sample cycle needs to run one
//                                 // extra to fill in the sliding average
//     boolean powerInitallyReleased = false;
//     uint32_t batteryVSense = 0;

//     while (sampleEnabled) {
//         threads.delay(intervalMS);

//         digitalWrite(BAT_SENSE_ENABLE_PIN, HIGH);

//         batteryVSense += analogRead(BAT_V_SENSE_PIN);

//         digitalWrite(BAT_SENSE_ENABLE_PIN, LOW);

//         pinMode(POWER_ENABLE_SENSE_PIN, INPUT_PULLUP);
//         delayMicroseconds(1);
//         bool powerPressed =
//             analogRead(POWER_ENABLE_SENSE_PIN) > POWER_PRESS_SENSE_LVL;
//         pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
//         pinMode(POWER_ENABLE_SENSE_PIN, HIGH); // Keep on
//         if (!powerPressed && !powerInitallyReleased)
//             powerInitallyReleased = true;
//         if (powerPressed && powerInitallyReleased) {
//             delay(200); // Verify its held down for 200 ms longer,
//                         // filter out accidental bumps
//             pinMode(POWER_ENABLE_SENSE_PIN, INPUT_PULLUP);
//             delayMicroseconds(1);
//             powerPressed =
//                 analogRead(POWER_ENABLE_SENSE_PIN) > POWER_PRESS_SENSE_LVL;
//             if (powerPressed) {
//                 pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
//                 pinMode(POWER_ENABLE_SENSE_PIN, LOW); // Turn off
//                 return;
//             } else {
//                 pinMode(POWER_ENABLE_SENSE_PIN,
//                         OUTPUT); // Keep on & ignore press
//                 pinMode(POWER_ENABLE_SENSE_PIN, HIGH);
//             }
//         }
//         if (sampleCounter == 9) {
//             batteryVSense /= 11; // double readings and 1 of 5 record is the
//                                  // rolling average from previous reading
//             Serial.print("***POWER*** Bat V:");
//             Serial.print(batteryVSense);
//             Serial.print(" Pwr Btn:");
//             Serial.println(powerPressed);
//             sampleCounter = 0;

//             if (batteryVSense < BAT_V_MIN_LVL) {
//                 Serial.print("***WARNING*** Bat V:");
//                 Serial.print(batteryVSense);
//                 Serial.print(" Turning Off");

//                 for (int i = 0; i < 10; i++) {
//                     digitalWrite(13, HIGH);
//                     delay(50);
//                     digitalWrite(13, LOW);
//                     delay(150);
//                 }
//                 pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
//                 pinMode(POWER_ENABLE_SENSE_PIN, LOW); // Turn off
//                 return;
//             }

//         } else {
//             sampleCounter++;
//         }
//     }
// }

BlinkTask blinkTask;

void setup() {
    // This must run ASAP to keep system on after power button is
    // released
    pinMode(POWER_ENABLE_SENSE_PIN, OUTPUT);
    digitalWrite(POWER_ENABLE_SENSE_PIN, HIGH); // Power Enable
    pinMode(POWER_LED_PIN, OUTPUT);

    Serial.begin(115200);
    timing_pair pair;
    pair = executor.getTimingPair(1, FrequencyUnitEnum::second);

    executor.schedule((RunnableTask *)&blinkTask, pair);

    pair.offsetMicros = (TIME_INT_t)50000ll;

    executor.schedule((RunnableTask *)&blinkTask, pair);
}

void loop() {
    uint32_t delayTime = executor.runSchedule();
    if (delayTime > 10000)
        delayTime = 10000;
    if (delayTime > MIN_MICRO_REST)
        delayMicroseconds(delayTime - MIN_MICRO_REST);
}
