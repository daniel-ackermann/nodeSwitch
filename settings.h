#ifndef SETTINGS_H
#define SETTINGS_H

// Pin an welchen die LEDs angeschlossen sind
#define LED_PIN                  2

// Anzahl der LEDs
#define NUMPIXELS               24

// Drehencoder
#define DREHENCODERPIN1          5
#define DREHENCODERPIN2          4
#define DISPLAY_LIGHT_POWER_PIN 16
#define BUTTON_PIN              12
#define ENTPRELL_ZEIT          600

// Piezo
#define PIEZO_PIN                3

// Webpage authentification
#define WWW_USER            "admin"
#define WWW_PASSWORD        "admin"

// Drehrichtung des Displays
#define DISPLAYROTATION          1

//Timezoneoffset - Dirty and wihtout changing hours summer/winter
#define HOUR_OFFSET 2

// Force AP for development
#define FORCE_ACCESSPOINT       false

// Logging
#define LOGGING_ACTIVE          true


/*******************************  PINOUT  *******************************************
 *                         _____________________                                   *
 *                        /                     \                                  *
 *                       /     WEMOS D1 MINI     \                                 *
 *                      /                         \                                *
 *             TFT_RST -|RST                 1  TX|-                               *
 *                      |A0  A0              3  RX|- Sound                         *
 *       TFT_BACKLIGHT -|D0  16              5  D1|- TFT                           *
 *                 TFT -|D5  14              4  D2|- Drehencoder                   *
 *              BUTTON -|D6  12              0  D3|- Drehencoder                   *
 *                 TFT -|D7  13              2  D4|- LED-Circle                    *
 *                 TFT -|D8  15                GND|- GND                           *
 *                3.3V -|_                      5V|- VCC                           *
 *                       _|                       |                                *
 *                      |_|Reset                  |                                *
 *                        |________<USB>__________|                                *
 *                                                                                 *
************************************************************************************/
/**
 * BUTTON D6 und TFT_BACKLIGHT D0 tauschen - GEHT NICHT! D0 kann kein Interrupt und kein PWM weshalb das Display auch nicht dimmbar war! 
 * LED-Circle D4 und Drehencoder D3 tauschen
 * https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 */
#define PROGMEM   ICACHE_RODATA_ATTR

// Höhe der leeren Fläche im Sprite über dem ersten Listeneintrag
#define DISPLAYOFFSET           66

// Zeilenhöhe
#define LINEHIGHT               54

// Scrollgeschwindigkeit
#define ROTATIONSPEEDFACTOR      4

// Breite vom Sprite
#define WIDTH                  200

// Websockets
#define MESSAGE_INTERVAL 30000
#define HEARTBEAT_INTERVAL 25000
// Eeprom
#define CFGSTART 0

#endif
