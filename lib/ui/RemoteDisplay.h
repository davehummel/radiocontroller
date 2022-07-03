#ifndef remote__display_H__
#define remote__display_H__

#include <Adafruit_GFX.h>


class RemoteUI {

  private:
    struct {
        uint16_t x = 190;
        uint16_t y = 0;
    } batteryProperties;

      struct {
        uint16_t x = 0;
        uint16_t y = 0;
    } receiverProperties;

    struct {
        uint16_t x = 0;
        uint16_t y = 18;
    } inputProperties;

    struct {
        uint16_t x = 0;
        uint16_t y = 184;
    } orientationProperties;

    struct {
        uint16_t x = 176;
        uint16_t y = 296;
    } motorProperties;

    Adafruit_GFX *display;

  public:
    RemoteUI();

    void setDisplay(Adafruit_GFX *_display) { display = _display; }

    void renderBackground();

    void setBatteryLocation(uint16_t x, uint16_t y);

    void setRadioStateLocation(uint16_t x, uint16_t y);

    void setInputLocation(uint16_t x, uint16_t y);

    void setOrientationLocation(uint16_t x, uint16_t y);

    void renderCTBattery(uint8_t ctLevel);

    void renderFCBattery(uint8_t fcLevel);

    void renderRadioState(uint8_t state, int8_t fcSNR, int8_t ctSNR );

    void renderInput(int8_t joyH, int8_t joyV,int8_t slideH, uint8_t slideV);
    
    void renderOrientation(float targetY,float actualY,float targetP,float actualP,float targetR,float actualR);

    void renderMotors(uint8_t ul,uint8_t ur,uint8_t bl, uint8_t br);
};

#endif