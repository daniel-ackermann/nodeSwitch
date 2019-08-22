#include "settings.h"

#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <Encoder.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <QList.h>
#include <ArduinoOTA.h>

extern "C" {
    #include "user_interface.h"
    #include "wpa2_enterprise.h"
}





typedef struct settingsType_t{
    char                    SSID[31]      = "";                    // SSID of WiFi
    char                    identity[31]  = "";                    // SSID of WiFi
    char                    password[31]  = "";                // Password of WiFi
    byte                    arduinoID     = 4;
    char                    QSIP[31]      = "";
    unsigned short int      QSPort        = 2222;
    byte                    wpa2          = 0;
} settingsType_t;


#include "settings.h"
#include "device.h"
#include "led.h"

#define DEG2RAD 0.0174532925

class QuickSwitch {
    private:
        const char*     wifissid;
        const char*     wifipassword;
        int             fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour);
        int             calculateColor(byte R, byte G, byte B);
        char            workingStatus               = 0;
        void            animateStatusBar();
        int             ypos                        = 0;
        // https://github.com/granadaxronos/120-SONG_NOKIA_RTTTL_RINGTONE_PLAYER_FOR_ARDUINO_UNO/blob/master/RTTTL_PLAYER/songs.h
        // const char * countdown = "The final countdown:d=4,o=5,b=125:p,8p,16b,16a,b,e,8p,16c6,16b,8c6,8b,a,8p,16c6,16b,c6,e,p,8p,16a,16g,8a,8g,8f#,8a,g.,16f#,16g,a.,16g,16a,8b,8a,8g,8f#,e,c6,2b.,16b,16c6,16b,16a,1b";
        // const char * imperial = "Imperial:d=4,o=5,b=100:e,e,e,8c,16p,16g,e,8c,16p,16g,e,p,b,b,b,8c6,16p,16g,d#,8c,16p,16g,e,8p";
        // const char * starwars = "Starwars:d=4,o=6,b=112:16c5,16p,16c5,16p,16c5,16p,f5,16p,c,16p,16p,16a5,16p,16g5,16p,f,16p,8c.5,16p,16a_5,16p,16a5,16p,16g5,16p,f,16p,8c.5,16p,16a_5,16p,16a5,16p,16g5,16p,f,16p,8c.5,16p,16a_5,16p,16a5,16p,16a_5,16p,2g5,8p,32c5,32p,8p,32c5,32p,32p,32c5,32p";
        // const char * mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
                
        // byte            songIndex       = 3;
        const char *    songNames[4]    = {"finalCountdown", "beethoven", "mario", "Customized Song"};
        const char *          song;
        // Only needed for displaying the status bar in the LCD
        // short int       shownStatus                 = 0;
        uint8_t         shownWifi                   = 0;
    public:
        void            displayWifiStrength();
        byte            useScreensaver              = 0;
        short int       selected                    = 0;                       //      index of devicelist
        short int       switchDevice                = 0;
        short int       deviceMode                  = 0;
        byte            ledBrightness               = 255;
        int             displayTimeout              = 10;
        byte            deviceCount                 = 0;
        int32_t         bgColor;
        TFT_eSPI        tft                         = TFT_eSPI();
        TFT_eSprite     sprite                      = TFT_eSprite(&tft);
        settingsType_t  config;
        QList<Device>   deviceList;
        Led             pixel;
        void            setYPosition(int position);
        int             getYPosition();
        const char *    getSong();
        void            saveConfig(settingsType_t cfg);
        void            loadConfig();
        boolean         setUpDeviceList(int counter);
        void            initTFT();
        void            initSprite(int height, int width);
        void            displayError(int code);
        void            removeError();
        void            updateLCD();
        void            updateLCD(bool wakeup);
        void            clearLCD();
        boolean         startWLAN();
};
