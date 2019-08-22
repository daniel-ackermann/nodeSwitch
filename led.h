#include <Adafruit_NeoPixel.h>
#include "settings.h"

/**
 *          8 = displayedValue
 *          15 = NUMPIXELS - displayedValue = 24 - 8 = 16
 *              |
 *       \17 16 15 14 13 12/
 *      18                 11
 *      19                 10
 *      20                  9
 *      21                  8
 *      22                  7
 *      23                  6
 *       / 0  1  2  3  4  5\
 * 
 *      NUMPIXELS       =     24
 *      displayedValue  = 0 - 23
 *      newValue        = 0 - 23
 * 
*/

#define LED_ANIMATION_SPEED 10

class Led {
    public:
        Adafruit_NeoPixel pixels                = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB);
    private:
        // Upper border
        byte displayedValue                      = 0;
        byte newValue                            = 0;
        // Lower border
        byte startIndex                          = 0;
        byte displayedStartIndex                 = 0;
        // Active
        bool sleeping                            = 0; // 1 = sleeping; 0 = awake
        // Color
        uint32_t displayedColor                 = pixels.Color(0,0,0);
        uint32_t newColor                       = pixels.Color(255,0,255);
        bool animate                            = false;
        unsigned long lastAnimation             = 0;
    public:
        void startAnimation();
        void powerOnAnimation();
        void setup();
        void updateAnimation();
        void sleep(boolean status);
        bool sleep();
        void setValue(float value, uint32_t color);
        void setValue(float value);
        void forceOff();
        void setColorDevice(float value);
        void setRelative(float value);
        int  calculateColor(float value);
};