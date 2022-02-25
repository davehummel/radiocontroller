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

    void renderTxBattery(uint8_t txLevel);

    void renderRxBattery(uint8_t rxLevel);

    void renderRadioState(uint8_t state, int8_t rxSNR, int8_t txSNR );
};

#endif