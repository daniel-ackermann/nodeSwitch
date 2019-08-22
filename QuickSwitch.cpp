#include "QuickSwitch.h"

void QuickSwitch::saveConfig(settingsType_t cfg){
    EEPROM.begin(sizeof(cfg));
    EEPROM.put( CFGSTART, cfg );
    EEPROM.commit();                      // Only needed for ESP8266 to get data written
    EEPROM.end();                         // Free RAM copy of structure
}

void QuickSwitch::loadConfig() {
    EEPROM.begin(sizeof(config));
    EEPROM.get( CFGSTART, config);
    EEPROM.end();
}

boolean QuickSwitch::setUpDeviceList(int counter){
    HTTPClient http;
    Serial.print(F("[HTTP] GET http://"));
    Serial.print(config.QSIP);
    Serial.print(F(":"));
    Serial.print(config.QSPort);
    Serial.print(F("/getSettings/"));
    Serial.println(config.arduinoID);
    if (http.begin("http://" + String(config.QSIP) + ":" + String(config.QSPort) + "/getSettings/" + config.arduinoID)){
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                Serial.print(F(" code: "));
                Serial.println(httpCode);
                Serial.print(F("[JSON] parse response: "));
                String payload = http.getString();
/*
                // Parse JSON object V6
                // const size_t capacity = JSON_ARRAY_SIZE(6) + 6*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(9) + 390;
                // DynamicJsonDocument jsonBuffer(capacity);
                // deserializeJson(jsonBuffer, payload);
                    DeserializationError error = deserializeJson(jsonBuffer, payload);
                    if (error) {
                        Serial.println("error: Fehler in den Einstellungen! JSON konnte nicht geparst werden!");
                        Serial.println(error.c_str());
                        return false;
                    }
                    Serial.println("successful");
*/
                const size_t capacity = JSON_ARRAY_SIZE(5) + 21*JSON_OBJECT_SIZE(3) + 890;
                DynamicJsonBuffer jsonBuffer(capacity);
                
                JsonObject& root            = jsonBuffer.parseObject(payload);
                JsonArray& devices          = root["devices"];
                if(root.success() && devices.success()){
                    Serial.println(F("done"));

                    bgColor                 = strtoul(root["bgColor"].as<char*>(), 0, 16);
                    Serial.print(F("Hintergrundfarbe:\t"));
                    Serial.println(bgColor , HEX);

                    ledBrightness           = root["ledBrightness"].as<int>();
                    if(ledBrightness < 1){
                        ledBrightness = 100;
                    }
                    Serial.print(F("Ledhelligkeit:\t\t"));
                    Serial.println(ledBrightness);

                    displayTimeout          = root["displayTimeout"].as<int>();
                    Serial.print(F("Timeout:\t\t\t"));
                    Serial.println(displayTimeout);
                    
                    useScreensaver          = root["useScreensaver"].as<byte>();
                    Serial.print(F("useScreensaver:\t\t"));
                    Serial.println(useScreensaver);


                    const static char *          songs[3]        = {
                        "The final Countdown:d=4,o=5,b=125:p,8p,16b,16a,b,e,8p,16c6,16b,8c6,8b,a,8p,16c6,16b,c6,e,p,8p,16a,16g,8a,8g,8f#,8a,g.,16f#,16g,a.,16g,16a,8b,8a,8g,8f#,e,c6,2b.,16b,16c6,16b,16a,1b",
                        "Beethoven:d=4,o=6,b=125:8e,8d_,8e,8d_,8e,8b5,8d,8c,a5,8p,8c5,8e5,8a5,b5,8p,8e5,8g_5,8b5,c,8p,8e5,8e,8d_,8e,8d_,8e,8b5,8d,8c,a5,8p,8c5,8e5,8a5,b5,8p,8e5,8c,8b5,a5",
                        "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6"
                    };

                    Serial.print(F("notification:\t"));
                    Serial.println(root["notification"].as<char*>());
                    if(root["notification"].as<char*>() == NULL || root["notification"].as<char*>() == "\0"){
                        song = songs[0];
                    }else{
                        song = strdup(root["notification"].as<char*>());
                        for(int i = 0; i < 3; i++){
                            if(strcmp( songNames[i], root["notification"].as<char*>() ) == 0){
                                song = songs[i];
                                break;
                            }
                        }
                    }

                    Serial.print(F("gelistete Geräte:\t"));
                    Serial.println(devices.size());
                    deviceCount = devices.size();

                    
                    initSprite(devices.size() * LINEHIGHT + 120 * 2, WIDTH);
                    int partDeg = pixel.pixels.numPixels() / devices.size();
                    for(int i = 0; i < devices.size(); i++){
                        const char* suffix = "";
                        if(devices[i]["suffix"].as<String>() != ""){
                            suffix = devices[i]["suffix"].as<char*>();
                        }
                        Device newDevice = *new Device(devices[i]["id"].as<char*>(), devices[i]["name"].as<char*>(), suffix, devices[i]["mode"].as<int>(), config.QSIP, config.QSPort);
                        // Device newDevice = *new Device(devices[i]["id"].as<char*>(), devices[i]["name"].as<char*>(), suffix, devices[i]["mode"].as<int>(), config.QSIP, config.QSPort, devices[i]["sendTimeout"].as<int>());
                        deviceList.push_back(newDevice);
                        // Write to sprite
                        if(devices[i]["mode"].as<int>() == 3){
                            // Variable mit Status
                            sprite.drawString(String(deviceList[i].getName()) + ": " + deviceList[i].getStatus() + String(deviceList[i].getSuffix()), 6, (LINEHIGHT * i) + LINEHIGHT + DISPLAYOFFSET, 1);
                        }else{
                            sprite.drawString(devices[i]["name"].as<char*>(), 6, (LINEHIGHT * i) + LINEHIGHT + DISPLAYOFFSET, 1);
                        }
                        for(int j = pixel.pixels.numPixels(); j >= pixel.pixels.numPixels() - partDeg * (i+1); j--){
                            pixel.pixels.setPixelColor(j, pixel.pixels.Color(0,150,0));
                        }
                        pixel.pixels.show();
                    }

                    pixel.forceOff();
                    return true;
                }
                Serial.println(F("error: Fehler in den Einstellungen! JSON konnte nicht geparst werden!"));
            }
        }
        Serial.print(F(" failed, error: "));
        Serial.println(httpCode);
        http.end();
        displayError(1);
        return false;
    }
    if(workingStatus != 1){
        displayError(1);
    }
    
    return setUpDeviceList(counter);
}

void QuickSwitch::initTFT(){
    tft.init();
    tft.setRotation(DISPLAYROTATION);
    tft.fillScreen(bgColor);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, bgColor);
    tft.setBitmapColor(TFT_WHITE, bgColor);
    if (SPIFFS.begin()) {
        Serial.println("\r\nSPIFFS available!");
        bool font_missing = false;
        if (SPIFFS.exists("/DejaVuSansCondensed28.vlw")    == false) font_missing = true;
        if (font_missing){
            Serial.println("\r\nFont missing in SPIFFS, did you upload it?");
        }else{
            tft.loadFont("DejaVuSansCondensed28");
        }
    }else{
        Serial.println("SPIFFS initialisation failed!");
    }
}

void QuickSwitch::initSprite(int height, int width){
    sprite.setColorDepth(1);
    sprite.createSprite(width, height); // Narrow and tall
    sprite.loadFont("DejaVuSansCondensed28");
    sprite.setTextDatum(MC_DATUM);
    // sprite.setWindow(0, 0, 100, 100);
}

void QuickSwitch::displayError(int code){
    workingStatus = code;
    String error = "";
    switch(code){
        case 1:      
            error = "No connection to Server";
            break;
        case 2:
            error = "No connection to WiFi";
            break;
        case 3:
            error = "Load Settings failed";
            break;
        case 4:
            error = "Connecting to WiFi";
            break;
        case 5:
            error = "Accesspoint started";
            break;
        case 6:
            error = "Loading Settings...";
            break;
        case 7:
            error = "Connected";
            break;
        case 8:
            // this->.tft.drawChar(0, 15, 33, TFT_RED, quickswitch.bgColor, 3); // Draw '!'
            tft.fillCircle(40, 25, 5, TFT_RED); // Draw circle
            return;
        default:
            workingStatus = -1;
            error = "not specified Error";
            break;
    }
    tft.fillScreen(bgColor);
    tft.unloadFont();
    tft.drawString(error,160, 90, 4);
    if(WiFi.localIP()){
        tft.drawString(WiFi.localIP().toString(), 160, 120, 4);
    }
    tft.loadFont("DejaVuSansCondensed28");
}

void QuickSwitch::removeError(){
    if(workingStatus == 8){
        tft.fillCircle(40, 25, 5, bgColor); // Draw circle
        displayWifiStrength(true);
    }
}

void QuickSwitch::setYPosition(int position){
    ypos = position;
}

int QuickSwitch::getYPosition(){
    return ypos;
}

void QuickSwitch::updateLCD(bool wakeup = false){
    // Update LCD
    // This is too slow!! Do not update the WifiSrength in the loop!
    // displayWifiStrength();
    tft.setTextColor(TFT_WHITE, bgColor);
    tft.setBitmapColor(TFT_WHITE, bgColor);
    tft.fillTriangle(50,115, 55,120, 50,125,TFT_WHITE);
    sprite.pushSprite(60, ypos);

    if(wakeup == false && pixel.sleep() == 1){
            return;
    }

    // Switch Display on
    digitalWrite(DISPLAY_LIGHT_POWER_PIN , 1);

    // Update LEDs
    switch(deviceList[selected].getDeviceMode()){
        // Prozent und An/Aus
        case 0:
        case 1:{
            pixel.setValue(deviceList[selected].getStatusFloat(), pixel.pixels.Color(255,0,255));
            break;
        }
        // RGB
        case 2:
            pixel.setColorDevice(deviceList[selected].getStatusFloat());
            break;
        // Variable
        case 3:
            pixel.setValue(0);
            break;
        // relative
        case 4:
            pixel.setRelative(deviceList[selected].getStatusFloat() * 100 - 50);
            break;

        default:
            Serial.println("Falscher Gerätetyp!!");
            break;
    }
}

void QuickSwitch::clearLCD(){
    tft.fillScreen(bgColor);
    tft.setTextColor(TFT_WHITE, bgColor);
    tft.setBitmapColor(TFT_WHITE, bgColor);
    shownWifi = 0;
}

// #########################################################################
// Draw circle segments
// #########################################################################

// x,y == coords of centre of circle
// start_angle = 0 - 359
// sub_angle   = 0 - 360 = subtended angle
// r = radius
// colour = 16 bit colour value

int QuickSwitch::fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour)
{
    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + sub_angle; i++) {

        // Calculate pair of coordinates for segment end
        int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
        int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;
    
        tft.fillTriangle(x1, y1, x2, y2, x, y, colour);
    
        // Copy segment end to sgement start for next segment
        x1 = x2;
        y1 = y2;
    }
}


int QuickSwitch::calculateColor(byte R, byte G, byte B){
    return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) );
}

boolean QuickSwitch::startWLAN(){
    /* WLAN */
    displayError(4);
    Serial.println("[WiFi] SSID:\t\t" + String(config.SSID));
    if(config.wpa2 > 0){
        // SSID to connect to
        // static const char* ssid = "kaenguroam-u";
        // Username for authentification
        // static const char* username = "4005";
        // Password for authentication
        // static const char* password = "vBWkcnnKP";
        Serial.println("[WiFi] use Enterprise");
        wifi_set_opmode(STATION_MODE);
        struct station_config wifi_config;
        memset(&wifi_config, 0, sizeof(wifi_config));
        strcpy((char*)wifi_config.ssid, config.SSID);
        // wifi_config.bssid_set = 0;
        wifi_station_set_config(&wifi_config);
        wifi_station_clear_cert_key();
        wifi_station_clear_enterprise_ca_cert();

        // wifi_station_set_enterprise_ca_cert((uint8*)cacert, sizeof(cacert));

        wifi_station_set_wpa2_enterprise_auth(1);
        wifi_station_set_enterprise_identity((uint8*)config.identity, strlen(config.identity));
        wifi_station_set_enterprise_username((uint8*)config.identity, strlen(config.identity));
        wifi_station_set_enterprise_password((uint8*)config.password, strlen(config.password));
        wifi_station_connect();
    }else{
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.SSID, config.password);
    }
    
    
    Serial.println(F("[WiFi] Waiting for connection and IP Address from DHCP"));
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("."));
        counter++;
        if(counter > 20){
            return false;
        }
        delay(500);
    }
    displayError(7);

    Serial.println(F("\n[WiFi] connected"));
    Serial.print(F("[WiFi] IP address:\t"));
    Serial.println(WiFi.localIP());
    return true;
}


void QuickSwitch::displayWifiStrength(bool force = false){
    // 100 / 92 * x
    int wifiNow = WiFi.RSSI();
    if(shownWifi != wifiNow || force){
        fillSegment(30, 30, -45, 90, 25, calculateColor('c', 'c', 'c'));
        fillSegment(30, 30, -45, 90, ( (100 - abs(wifiNow)) * 0.25), calculateColor('3', '3', '3'));
        shownWifi = wifiNow;
    }
}

/*
// for usage uncomment int shownStatus = 0; in quickswitch.h!
void QuickSwitch::animateStatusBar(){
    switch(deviceList[selected].getDeviceMode()){
        // Prozent und An/Aus
        case 0:
        case 1:{
            int newValue = (int)(this->deviceList[selected].getStatusFloat() * 308);
            if(shownStatus < newValue){
                // Wachsen
                this->tft.fillRect(5, 228, newValue + 2, 10, TFT_WHITE);
                shownStatus = newValue;
            }else if(shownStatus > newValue){
                // Schrumpfen
                this->tft.fillRect(newValue + 2, 228, 314, 10, this->bgColor);
                shownStatus = newValue;
            }else{
                // Stagnieren
                this->tft.fillRect(5, 228, shownStatus + 2, 10, TFT_WHITE);
            }
            break;
        }

        // RGB
        case 2:
            this->tft.fillRect(5, 228, 308, 10, this->pixel.calculateColor(this->deviceList[selected].getStatusFloat()));
            break;

        // Variable
        case 3:
            shownStatus = 0;
            tft.fillRect(7, 228, shownStatus, 10, bgColor);
            break;

        // relative
        case 4:
            // this->tft.drawString((String)deviceList[selected].getStatusFloat(), 20, 30, 1); // Dies kann abstürze verursachen!!
            break;

        default:
            Serial.println("Falscher Gerätetyp!!");
            break;
    }
}
*/
const char * QuickSwitch::getSong(){
    return song;
}