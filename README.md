# nodeSwitch

nodeSwitch ist eine an der Wand angebrachte Fernbedienung welche die Möglichkeit bietet unterschiedlichste Geräte zu steuern und
gleichzeitig auch Funktionen zur Benachrichtigung von Bewohnern eines Hauses beinhaltet.

nodeSwitch ist darauf ausgerichtet mit QuickSwitch2.0 zu harmonieren.

Das Gerät stellt eine Webseite zum konfigurieren des WLANs und der Verbindung zu QuickSwitch bereit. Von dort lädt es beim start
alle weiteren Einstellungen.

Darüber hinaus existieren alle folgenden REST-Endpunkte:

```
GET /                       // Main Page with some basic information
GET /config                 // Settings Page to configure all important things
GET /reset                  // call this to restart the system
GET /update                 // here you can upload new *.bin files to update the system
GET /ntpGetTime             // call this will update the time of the system, but it should update it self (every 10 Min)
GET /showFont               // this endpoint displays all available Chars of the loaded font
GET /wakeup                 // call this to wake the system up from sleeping
GET /index.css              // serves the Stylesheet for the webpages
```

Für die Kommunikation mit QuickSwitch2.0 existieren darüber hinaus noch folgende:
```
POST /action                // Is used to notify for an changed status of a device
POST /alert                 // Is used to display Alerts
POST /variable              // Is used to update the value of a displayed Variable
```

Get started:
You need to install the ESP-Boards in Version 2.5.0 and the following Libraries:                                    // Version > 2.5.0 are not working jet!
```
Encoder                     // Version: 1.4.1  by Paul Stoffregen   https://github.com/PaulStoffregen/Encoder
TFT_eSPI                    // Version: 1.4.16 by Bodmer            https://github.com/Bodmer/TFT_eSPI/
Adafruit NeoPixel           // Version: 1.2.4  by Adafruit          https://github.com/adafruit/Adafruit_NeoPixel
ArduinoJSON                 // Version: 5.13.5 by Benoit Blanchon   https://arduinojson.org/                        // Version 6 ist not working jet!
QList                       // Version: 0.6.7  by Martin Dagarin    https://github.com/SloCompTech/QList
```