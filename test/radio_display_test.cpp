#include <Arduino.h>
#include <SPI.h>

#include "FDOS_LOG.h"
#include "VMExecutor.h"
#include "VMTime.h"

#include "RadioTask.h"
#include <RadioLib.h>
#include <U8g2lib.h>

// U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/DISP_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

SX1276 radio(new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN, SPI, SPISettings(SPI_RADIO_FREQ, MSBFIRST, SPI_MODE0)));
void beginReceive(void) { radio.startReceive(); }

Logger FDOS_LOG(&Serial);

VMExecutor executor;

RadioTask radioTask(&radio);

uint8_t spiBlock = 0;

class DisplayRadioTestTask : RadioAction, RunnableTask {
    uint16_t boxX = 0, boxY = 0, velX = 1, velY = 1;

    TIME_INT_t lastTime = microsSinceEpoch();

    void run(TIME_INT_t time) {
        if (boxX % 100 == 0) {
            // u8g2.setDrawColor(1);
            // u8g2.drawBox(10, 100, 80, 54);
            // u8g2.setDrawColor(0);
            // u8g2.setCursor(10, 100);
            // u8g2.print(boxX);
            // u8g2.setCursor(10, 116);
            // u8g2.print(boxY);
            // u8g2.setCursor(10, 132);
            // u8g2.print((long)(time - lastTime)/1000);
            lastTime = time;
            boxY = 0;
            spiBlock = 1;
            // u8g2.sendBuffer();
        }
        if (spiBlock == 2)
            radioTask->interruptTriggered();
        spiBlock = 0;
        boxX++;
        requestSend();
    }

    void onStart() { executor.schedule((RunnableTask *)this, executor.getTimingPair(30, FrequencyUnitEnum::milli)); }

    void onStop() {}

    void onReceive(uint8_t length, uint8_t *data) {
        boxY = (data[3] << 8) + data[2];
        digitalWrite(LED_PIN, false);
        FDOS_LOG.println("Received!");
    }

    // returns length of data to send
    uint8_t onSendReady(uint8_t *data) {
        FDOS_LOG.println("Sent!");
        data[0] = boxX & 0xFF;
        data[1] = (boxX & 0xFF00) >> 8;
        data[2] = boxY & 0xFF;
        data[3] = (boxY & 0xFF00) >> 8;
        return 4;
    }

} displayTask;

void radioInterrupt(void) {
    if (spiBlock == 1)
        spiBlock = 2;
    else
        radioTask.interruptTriggered();
}

void setup(void) {
    delay(10000);
    // SPI.setSCK(SPI_CLK_PIN);
    SPI.begin();
    // PowerEnable pin must be enabled ASAP to keep system on after power is
    // released
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, true);

    Serial.begin(921600);

    Serial.println("Display Starting ...");
    // u8g2.setBusClock(SPI_DISP_FREQ);
    // u8g2.begin();
    // u8g2.clearBuffer();
    // u8g2.setDrawColor(1);
    // u8g2.drawBox(0, 0, 240, 400);
    // u8g2.setDrawColor(0); // in order to make content visable
    // u8g2.setFont(u8g2_font_timB14_tr);
    // // u8g2.setFontRefHeightExtendedText();
    // u8g2.setFontPosTop();
    // // u8g2.setFontDirection(0);

    // u8g2.drawStr(0, 0, "Display Ready!");
    // u8g2.drawStr(0, 20, "Starting Radio ...");
    // u8g2.sendBuffer();

    int state = radio.begin(915, 250, 9,7,18,17); //-23dBm
    if (state == RADIOLIB_ERR_NONE) {
        FDOS_LOG.println("success!");
    } else {
        FDOS_LOG.print("SX1276 failed. Code:");
        FDOS_LOG.println(state);
        // u8g2.drawStr(0, 40, "Radio Failed!");
           // u8g2.sendBuffer();
        while (true) {
            digitalWrite(LED_PIN, true);
            delay(100);
            digitalWrite(LED_PIN, false);
            delay(200);
        }
    }
    // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    // NOTE: 20 dBm value allows high power operation, but transmission
    //       duty cycle MUST NOT exceed 1%
    if (radio.setOutputPower(17) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        FDOS_LOG.println("Invalid Power!");
        //   u8g2.drawStr(0, 40, "Invalid Power!");
             // u8g2.sendBuffer();
        while (true) {
            digitalWrite(LED_PIN, true);
            delay(200);
            digitalWrite(LED_PIN, false);
            delay(100);
        }
    }

    radio.setRfSwitchPins(RADIO_RX_EN_PIN, RADIO_TX_EN_PIN);

    radio.setDio0Action(radioInterrupt);
    radio.setDio1Action(radioInterrupt);

    // u8g2.drawStr(0, 40, "Radio Ready!");
   // u8g2.sendBuffer();
    delay(2000);

    digitalWrite(LED_PIN, false);

    executor.schedule((RunnableTask *)&radioTask, executor.getTimingPair(RADIO_INTERVAL_MILLIS, FrequencyUnitEnum::milli));

    radioTask.addAction((RadioAction *)&displayTask);
}

void loop(void) {
    uint32_t delayTime = executor.runSchedule();
    // You may want to allow the loop to finish and avoid long delays
    //    if you are using background arduino features
    if (delayTime > 100000)
        delayTime = 100000;
    if (delayTime > MIN_MICRO_REST) {
        delayMicroseconds(delayTime - MIN_MICRO_REST);
    }
}

/*
  GraphicsTest.ino
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.
  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// #include <Arduino.h>
// #include <SPI.h>
// #include <U8g2lib.h>

// U8G2_LS027B7DH01_M0_400X240_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/TFT_CS_PIN, /* dc=*/U8X8_PIN_NONE, /* reset=*/U8X8_PIN_NONE);

// void u8g2_prepare(void) {
//     u8g2.setFont(u8g2_font_6x10_tf);
//     u8g2.setFontRefHeightExtendedText();
//     u8g2.setDrawColor(1);
//     u8g2.setFontPosTop();
//     u8g2.setFontDirection(0);
// }

// void u8g2_box_frame(uint8_t a) {
//     u8g2.drawStr(0, 0, "drawBox");
//     u8g2.drawBox(5, 10, 20, 10);
//     u8g2.drawBox(10 + a, 15, 30, 7);
//     u8g2.drawStr(0, 30, "drawFrame");
//     u8g2.drawFrame(5, 10 + 30, 20, 10);
//     u8g2.drawFrame(10 + a, 15 + 30, 30, 7);
// }

// void u8g2_disc_circle(uint8_t a) {
//     u8g2.drawStr(0, 0, "drawDisc");
//     u8g2.drawDisc(10, 18, 9);
//     u8g2.drawDisc(24 + a, 16, 7);
//     u8g2.drawStr(0, 30, "drawCircle");
//     u8g2.drawCircle(10, 18 + 30, 9);
//     u8g2.drawCircle(24 + a, 16 + 30, 7);
// }

// void u8g2_r_frame(uint8_t a) {
//     u8g2.drawStr(0, 0, "drawRFrame/Box");
//     u8g2.drawRFrame(5, 10, 40, 30, a + 1);
//     u8g2.drawRBox(50, 10, 25, 40, a + 1);
// }

// void u8g2_string(uint8_t a) {
//     u8g2.setFontDirection(0);
//     u8g2.drawStr(30 + a, 31, " 0");
//     u8g2.setFontDirection(1);
//     u8g2.drawStr(30, 31 + a, " 90");
//     u8g2.setFontDirection(2);
//     u8g2.drawStr(30 - a, 31, " 180");
//     u8g2.setFontDirection(3);
//     u8g2.drawStr(30, 31 - a, " 270");
// }

// void u8g2_line(uint8_t a) {
//     u8g2.drawStr(0, 0, "drawLine");
//     u8g2.drawLine(7 + a, 10, 40, 55);
//     u8g2.drawLine(7 + a * 2, 10, 60, 55);
//     u8g2.drawLine(7 + a * 3, 10, 80, 55);
//     u8g2.drawLine(7 + a * 4, 10, 100, 55);
// }

// void u8g2_triangle(uint8_t a) {
//     uint16_t offset = a;
//     u8g2.drawStr(0, 0, "drawTriangle");
//     u8g2.drawTriangle(14, 7, 45, 30, 10, 40);
//     u8g2.drawTriangle(14 + offset, 7 - offset, 45 + offset, 30 - offset, 57 + offset, 10 - offset);
//     u8g2.drawTriangle(57 + offset * 2, 10, 45 + offset * 2, 30, 86 + offset * 2, 53);
//     u8g2.drawTriangle(10 + offset, 40 + offset, 45 + offset, 30 + offset, 86 + offset, 53 + offset);
// }

// void u8g2_ascii_1() {
//     char s[2] = " ";
//     uint8_t x, y;
//     u8g2.drawStr(0, 0, "ASCII page 1");
//     for (y = 0; y < 6; y++) {
//         for (x = 0; x < 16; x++) {
//             s[0] = y * 16 + x + 32;
//             u8g2.drawStr(x * 7, y * 10 + 10, s);
//         }
//     }
// }

// void u8g2_ascii_2() {
//     char s[2] = " ";
//     uint8_t x, y;
//     u8g2.drawStr(0, 0, "ASCII page 2");
//     for (y = 0; y < 6; y++) {
//         for (x = 0; x < 16; x++) {
//             s[0] = y * 16 + x + 160;
//             u8g2.drawStr(x * 7, y * 10 + 10, s);
//         }
//     }
// }

// void u8g2_extra_page(uint8_t a) {
//     u8g2.drawStr(0, 0, "Unicode");
//     u8g2.setFont(u8g2_font_unifont_t_symbols);
//     u8g2.setFontPosTop();
//     u8g2.drawUTF8(0, 24, "☀ ☁");
//     switch (a) {
//     case 0:
//     case 1:
//     case 2:
//     case 3:
//         u8g2.drawUTF8(a * 3, 36, "☂");
//         break;
//     case 4:
//     case 5:
//     case 6:
//     case 7:
//         u8g2.drawUTF8(a * 3, 36, "☔");
//         break;
//     }
// }

// #define cross_width 24
// #define cross_height 24
// static const unsigned char cross_bits[] U8X8_PROGMEM = {
//     0x00, 0x18, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x81, 0x00, 0x00, 0x81, 0x00,
//     0xC0, 0x00, 0x03, 0x38, 0x3C, 0x1C, 0x06, 0x42, 0x60, 0x01, 0x42, 0x80, 0x01, 0x42, 0x80, 0x06, 0x42, 0x60, 0x38, 0x3C, 0x1C, 0xC0, 0x00, 0x03,
//     0x00, 0x81, 0x00, 0x00, 0x81, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x18, 0x00,
// };

// #define cross_fill_width 24
// #define cross_fill_height 24
// static const unsigned char cross_fill_bits[] U8X8_PROGMEM = {
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x64, 0x00, 0x26, 0x84, 0x00, 0x21, 0x08, 0x81, 0x10, 0x08, 0x42, 0x10, 0x10, 0x3C, 0x08,
//     0x20, 0x00, 0x04, 0x40, 0x00, 0x02, 0x80, 0x00, 0x01, 0x80, 0x18, 0x01, 0x80, 0x18, 0x01, 0x80, 0x00, 0x01, 0x40, 0x00, 0x02, 0x20, 0x00, 0x04,
//     0x10, 0x3C, 0x08, 0x08, 0x42, 0x10, 0x08, 0x81, 0x10, 0x84, 0x00, 0x21, 0x64, 0x00, 0x26, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// };

// #define cross_block_width 14
// #define cross_block_height 14
// static const unsigned char cross_block_bits[] U8X8_PROGMEM = {
//     0xFF, 0x3F, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0xC1, 0x20,
//     0xC1, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0xFF, 0x3F,
// };

// void u8g2_bitmap_overlay(uint8_t a) {
//     uint8_t frame_size = 28;

//     u8g2.drawStr(0, 0, "Bitmap overlay");

//     u8g2.drawStr(0, frame_size + 12, "Solid / transparent");
//     u8g2.setBitmapMode(false /* solid */);
//     u8g2.drawFrame(0, 10, frame_size, frame_size);
//     u8g2.drawXBMP(2, 12, cross_width, cross_height, cross_bits);
//     if (a & 4)
//         u8g2.drawXBMP(7, 17, cross_block_width, cross_block_height, cross_block_bits);

//     u8g2.setBitmapMode(true /* transparent*/);
//     u8g2.drawFrame(frame_size + 5, 10, frame_size, frame_size);
//     u8g2.drawXBMP(frame_size + 7, 12, cross_width, cross_height, cross_bits);
//     if (a & 4)
//         u8g2.drawXBMP(frame_size + 12, 17, cross_block_width, cross_block_height, cross_block_bits);
// }

// void u8g2_bitmap_modes(uint8_t transparent) {
//     const uint8_t frame_size = 24;

//     u8g2.drawBox(0, frame_size * 0.5, frame_size * 5, frame_size);
//     u8g2.drawStr(frame_size * 0.5, 50, "Black");
//     u8g2.drawStr(frame_size * 2, 50, "White");
//     u8g2.drawStr(frame_size * 3.5, 50, "XOR");

//     if (!transparent) {
//         u8g2.setBitmapMode(false /* solid */);
//         u8g2.drawStr(0, 0, "Solid bitmap");
//     } else {
//         u8g2.setBitmapMode(true /* transparent*/);
//         u8g2.drawStr(0, 0, "Transparent bitmap");
//     }
//     u8g2.setDrawColor(0); // Black
//     u8g2.drawXBMP(frame_size * 0.5, 24, cross_width, cross_height, cross_bits);
//     u8g2.setDrawColor(1); // White
//     u8g2.drawXBMP(frame_size * 2, 24, cross_width, cross_height, cross_bits);
//     u8g2.setDrawColor(2); // XOR
//     u8g2.drawXBMP(frame_size * 3.5, 24, cross_width, cross_height, cross_bits);
// }

// uint8_t draw_state = 0;

// void draw(void) {
//     u8g2_prepare();
//     switch (draw_state >> 3) {
//     case 0:
//         u8g2_box_frame(draw_state & 7);
//         break;
//     case 1:
//         u8g2_disc_circle(draw_state & 7);
//         break;
//     case 2:
//         u8g2_r_frame(draw_state & 7);
//         break;
//     case 3:
//         u8g2_string(draw_state & 7);
//         break;
//     case 4:
//         u8g2_line(draw_state & 7);
//         break;
//     case 5:
//         u8g2_triangle(draw_state & 7);
//         break;
//     case 6:
//         u8g2_ascii_1();
//         break;
//     case 7:
//         u8g2_ascii_2();
//         break;
//     case 8:
//         u8g2_extra_page(draw_state & 7);
//         break;
//     case 9:
//         u8g2_bitmap_modes(0);
//         break;
//     case 10:
//         u8g2_bitmap_modes(1);
//         break;
//     case 11:
//         u8g2_bitmap_overlay(draw_state & 7);
//         break;
//     }
// }

// void setup(void) {
//     SPI.setSCK(SPI_CLK_PIN);
//     u8g2.setBusClock(8000000);
//     u8g2.begin();
// }

// void loop(void) {
//     // picture loop
//     u8g2.clearBuffer();
//     draw();
//     u8g2.sendBuffer();

//     // increase the state
//     draw_state++;
//     if (draw_state >= 12 * 8)
//         draw_state = 0;

//     // deley between each page
//     // delay(100);
// }