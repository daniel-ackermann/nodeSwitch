#include <TFT_eSPI.h>
#include "settings.h"

class Screensaver{
    private:
        boolean activeNow   = false;
        char *displayedValue = "00:00";
        byte mode         = 0;
        TFT_eSPI* tft;
    public:
        Screensaver(TFT_eSPI* tft);
        void update(char value[8]);
        void show();
        boolean hide(uint32_t color);
        boolean active();
        void setActive(boolean status);
        void setMode(byte mode);
};