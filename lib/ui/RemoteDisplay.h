#ifndef remote__display_H__
#define remote__display_H__

#include <Adafruit_GFX.h>
#include <RadioTask.h>

class RemoteUI:RadioUI {

  private:
    struct {
        uint16_t x = 190;
        uint16_t y = 0;
    } batteryProperties;

      struct {
        uint16_t x = 0;
        uint16_t y = 0;
    } receiverProperties;

    Adafruit_GFX *display;

  public:
    RemoteUI();

    void setDisplay(Adafruit_GFX *_display) { display = _display; }

    void renderBackground();

    void setBatteryLocation(uint16_t x, uint16_t y);

    void setRadioStateLocation(uint16_t x, uint16_t y);

    void renderCTBattery(uint8_t ctLevel);

    void renderFCBattery(uint8_t fcLevel);

    void renderRadioState(uint8_t state, int8_t fcSNR, int8_t ctSNR );
};

#endif