#include "QuickSwitch.h"
#include "screensaver.h"
#include <NonBlockingRtttl.h>           // https://github.com/end2endzone/NonBlockingRTTTL

#include <ESP8266HTTPUpdateServer.h>

/**
 * 
 * Software
 * erledigt Verstärker funktion +10|-10
 * erledigt Alerts
 * erledigt wpa2 identity webpage
 * erledigt wpa2 use webpagedata
 * 
 * Version 2:
 *      Display fest machen
 *      LED-Leuchtet, Pin als Eingang nutzen
 *      USB-Pins rausführen??? Kann muss nicht
 *      Piezzo!
 * 
 * 
 * Stromaufnahme:
 * LED: 60mA
 * 0.06A * 24 = 1,44A
 * ESP: 500mA
 * Display: Keine info: Annahme: 500mA
 * 
 * = 2,44A
 * 
 **/


// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
QuickSwitch         quickswitch;
Screensaver         screensaver(&quickswitch.tft);
Encoder             myEnc(DREHENCODERPIN1, DREHENCODERPIN2);
ESP8266WebServer    server(80);
ESP8266HTTPUpdateServer httpUpdater;


// Warum keine PPD?
unsigned long           alteZeit                = 0;
unsigned long           lastAction              = 0;
long                    oldPosition             = 0;
long                    oldPositionTick         = 0;
unsigned int            oldPercent              = 0;
int                     wlanReconnectCount      = 0;
boolean                 updateActive            = false;
int                     alertActive             = -1;
const static String     resetReason             = ESP.getResetReason();

// Time
byte                    hh                      = 0;
byte                    mi                      = 0;
byte                    ss                      = 0; // hours, minutes, seconds
byte                    hhUptime                = 0;
byte                    miUptime                = 0;
byte                    ssUptime                = 0; // hours, minutes, seconds
byte                    dddd                    = 0; // days (for if you want a calendar)
unsigned long int       lastTick                = 0; // time that the clock last "ticked"
char uptime[20];                                     // for output
// char *localTime;
char localTime[8];
unsigned short int      localPort               = 2390;

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress               timeServerIP;                                   // time.nist.gov NTP server address
const byte              NTP_PACKET_SIZE         = 48;                   // NTP time stamp is in the first 48 bytes of the message
byte                    packetBuffer[ NTP_PACKET_SIZE ];                //buffer to hold incoming and outgoing packets
const char*             ntpServerName           = "time.nist.gov";
unsigned long int       lastUpdate              = 0;




/***
/device/id/status
/variable/id/status
/alert/id/title/message/color ??
/config/key/value
    keys[QSIP,QSPort,ardunioID,SSID,password]
/reset
/showFont/Sekunden
***/

const char htmlHeader[] PROGMEM           = "<html>"
                                                "<head>"
                                                    "<meta http-equiv='content-type' content='text/html; charset=UTF-8'>"
                                                    "<title>nodeSwitch</title>"
                                                    "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                                                    "<link rel='stylesheet' href='./index.css' />"
                                                "</head>"
                                                "<body>"
                                                    "<div id='content'>"
                                                        "<h1>nodeSwitch<div>Die neue Generation Lichtschalter</div></h1>";
const char htmlFooter[] PROGMEM                       = "<div id='footer'>"
                                                            "<a target='_blank' href='https://github.com/dede53'>Developed by dede53</a> Version:1.1.0"
                                                        "</div>"
                                                    "</div>"
                                                "</body>"
                                            "</html>";


void setup() {

    quickswitch.loadConfig();
    quickswitch.initTFT();

    /* Serial */
    Serial.begin(115200);
    Serial.println("Starting..");

    Serial.print(F("[ESP] Resetreason:\t"));
    Serial.println(resetReason);

    system_update_cpu_freq(SYS_CPU_160MHZ);
    Serial.print(F("[ESP] cpu_freq:\t\t"));
    Serial.println(system_get_cpu_freq());

    // Serial.print("[ESP] system_get_free_heap_size:\t");
    // Serial.println(system_get_free_heap_size());

    Serial.print(F("[ESP] SDK-Version:\t"));
    Serial.println(system_get_sdk_version());

    // Rotationcoder
    pinMode(DREHENCODERPIN1, INPUT_PULLUP);
    pinMode(DREHENCODERPIN2, INPUT_PULLUP);

    // Button
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    quickswitch.pixel.setup();
    quickswitch.pixel.forceOff();

    quickswitch.tft.loadFont("DejaVuSansCondensed28");

    // Displaylight
    pinMode(DISPLAY_LIGHT_POWER_PIN, OUTPUT);
    digitalWrite(DISPLAY_LIGHT_POWER_PIN, 1);

    if(digitalRead(BUTTON_PIN) == HIGH && quickswitch.startWLAN() && FORCE_ACCESSPOINT == false){
        // quickswitch.pixel.startAnimation();
        quickswitch.displayError(6);
        int counter = 0;
        while(!quickswitch.setUpDeviceList(0) && counter < 5){
            quickswitch.tft.unloadFont();
            quickswitch.tft.drawString("Next time trying in:", 160, 120, 4);
            for(int i = 5; i > 0; i--){
                quickswitch.tft.drawNumber(i, 160, 150, 4);
                delay(1000);
            }
            counter++;
        }

        screensaver.setMode(quickswitch.useScreensaver);
        quickswitch.tft.loadFont("DejaVuSansCondensed28");
        if(counter < 5){
            quickswitch.pixel.pixels.setBrightness(quickswitch.ledBrightness);
            digitalWrite(DISPLAY_LIGHT_POWER_PIN, 1);
            quickswitch.clearLCD();
            quickswitch.displayWifiStrength(false);
            quickswitch.updateLCD(false);

            attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), sendStatusToQuickSwitch, FALLING);
        }else{
            detachInterrupt(digitalPinToInterrupt(DREHENCODERPIN1));
            detachInterrupt(digitalPinToInterrupt(DREHENCODERPIN2));
            quickswitch.displayError(3);
        }
    }else{
        setUpAccesspoint();
    }
    
    lastAction = millis();
    Serial.println(F("[NTP] Starting UDP"));
    udp.begin(localPort);

    sprintf(uptime,"%d Tagen %02d:%02dh",dddd, hhUptime, miUptime);
    sprintf(localTime, " %02d:%02d ", hh, mi);

    if (MDNS.begin("nodeSwitch")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println(F("[MDNS] responder started"));
    }

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("nodeSwitch");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    
    ArduinoOTA.onStart([]() {
        // Deaktivate Interrupt for a smooth update
        detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
        detachInterrupt(digitalPinToInterrupt(DREHENCODERPIN1));
        detachInterrupt(digitalPinToInterrupt(DREHENCODERPIN2));
        // To avoid
        updateActive = true;

        screensaver.hide(quickswitch.bgColor);
        quickswitch.tft.fillScreen(quickswitch.bgColor);
        quickswitch.tft.setTextColor(TFT_WHITE, quickswitch.bgColor);
        quickswitch.tft.setTextDatum(MC_DATUM);
        quickswitch.tft.drawString( "Update:" , 160, 90, 1);
        quickswitch.tft.drawString( "%" , 175, 120, 1);
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        quickswitch.pixel.sleep(true);
        digitalWrite(DISPLAY_LIGHT_POWER_PIN, 1);
    });
    ArduinoOTA.onEnd([](){
        quickswitch.tft.setTextDatum(MC_DATUM);
        quickswitch.tft.drawString("Successfull finished", 160, 90, 4);
        quickswitch.tft.drawString("Restarting...", 160, 120, 4);
        delay(500);
        digitalWrite(DISPLAY_LIGHT_POWER_PIN, 0);
        ESP.restart();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
        int percent = progress / (total / 100);
        if(percent == oldPercent){
            return;
        }
        oldPercent = percent;
        quickswitch.tft.setTextDatum(MR_DATUM);
        quickswitch.tft.drawString( String(percent) , 160, 120, 1);
        quickswitch.tft.fillRect(5, 228, percent * 3.08 + 2, 10, TFT_WHITE);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        // Serial.print("Error:" + error);
        if (error == OTA_AUTH_ERROR) {
            quickswitch.tft.drawString( "Auth Failed" , 160, 120, 1);
        } else if (error == OTA_BEGIN_ERROR) {
            quickswitch.tft.drawString( "Begin Failed" , 160, 120, 1);
        } else if (error == OTA_CONNECT_ERROR) {
            quickswitch.tft.drawString( "Connect Failed" , 160, 120, 1);
        } else if (error == OTA_RECEIVE_ERROR) {
            quickswitch.tft.drawString( "Receive Failed" , 160, 120, 1);
        } else if (error == OTA_END_ERROR) {
            quickswitch.tft.drawString( "End Failed" , 160, 120, 1);
        }
        delay(1000);
        ESP.restart();
    });
    ArduinoOTA.begin();
    setupWebserver();

    ntpGetTime();

    Serial.print(F("sizeofQuickswitch:\t"));
    Serial.println(sizeof(quickswitch));
    Serial.print(F("\tsizeofQuickswitch.pixel:\t"));
    Serial.println(sizeof(quickswitch.pixel));
    Serial.print(F("\tsizeofQuickswitch.tft:\t\t"));
    Serial.println(sizeof(quickswitch.tft));
    Serial.print(F("sizeofServer:\t\t"));
    Serial.println(sizeof(server));
    Serial.print(F("sizeofArduinoOTA:\t"));
    Serial.println(sizeof(ArduinoOTA));
    Serial.print(F("sizeofScreensaver:\t"));
    Serial.println(sizeof(screensaver));
}

void loop() {
    // Serial.print(ESP.getHeapFragmentation());
    // Serial.print("\t\t");
    // Serial.println(system_get_free_heap_size());
    
    server.handleClient();
    quickswitch.pixel.updateAnimation();
    MDNS.update();
    drehencoder(myEnc.read());
    ArduinoOTA.handle();
    udpServerLoop();
    rtttl::play();


    /* Verarbeitung des Interrupts */
    if(quickswitch.switchDevice != 0 && updateActive == false){
        quickswitch.switchDevice = 0;
        if(dismissAlert()){
            return;
        }
        switch(quickswitch.deviceList[quickswitch.selected].getDeviceMode()){
            case 1:   //SetPercent
                if(quickswitch.deviceMode == 1){
                    if(quickswitch.deviceList[quickswitch.selected].turn()){
                        quickswitch.deviceMode = 0;
                    }else{
                        quickswitch.displayError(8);
                        return;
                    }
                }else{
                    // selectStatus
                    quickswitch.deviceMode = 1;
                }
                break;
            case 2:   //setColorDevice
                if(quickswitch.deviceMode == 2){
                    if(quickswitch.deviceList[quickswitch.selected].turn()){
                        quickswitch.deviceMode = 0;
                    }else{
                        quickswitch.displayError(8);
                        return;
                    }
                }else{
                    // selectStatus
                    quickswitch.deviceMode = 2;
                }
                break;
            case 3:
                // Tue nichts, da Variable.
                break;
            case 4:   //setRelative
                if(quickswitch.deviceMode == 4){
                    if(quickswitch.deviceList[quickswitch.selected].turn()){
                        quickswitch.deviceList[quickswitch.selected].setStatus("0.50");
                        quickswitch.deviceMode = 0;
                    }else{
                        quickswitch.displayError(8);
                        return;
                    }
                }else{
                    quickswitch.deviceMode = 4;
                }
                break;
            default:  //SwitchDevice
                quickswitch.deviceMode = 0;
                int newStatus = 1;
                if(quickswitch.deviceList[quickswitch.selected].getStatusInt()){
                    newStatus = 0;
                }
                quickswitch.deviceList[quickswitch.selected].setStatus(newStatus);
                if(quickswitch.deviceList[quickswitch.selected].turn()){
                    quickswitch.deviceList[quickswitch.selected].setStatus(newStatus);
                }else{
                    quickswitch.displayError(8);
                    return;
                }
                break;
        }

        if(screensaver.hide(quickswitch.bgColor)){
            quickswitch.clearLCD();
        }
        quickswitch.removeError();
        quickswitch.updateLCD(true);

        quickswitch.displayWifiStrength(true);
    }
    /*** Display auto off ***/
    if(lastAction + quickswitch.displayTimeout * 1000 < millis() && quickswitch.pixel.sleep() == 0 && alertActive < 0 && screensaver.active() == false) {
        quickswitch.pixel.sleep(true);
        screensaver.show();
    }

    /*** Timecounter ***/
    if ((millis() - lastTick * 1000) >= 1000) {
        lastTick++;
        ss++;
        if(ss < 60){
            return;
        }
        ss = 0;

        // Following Code runs every Minute:

        // Update Wifi Strength
        if(!screensaver.active()){
            quickswitch.displayWifiStrength(false);
        }

        // Update time and uptime
        miUptime++;
        if (miUptime>59) {
            miUptime=0;
            hhUptime++;
        }
        if (hhUptime>23) {
            hhUptime=0;
            dddd++;
        }
        sprintf(uptime, "%d Tagen %02d:%02dh",dddd, hhUptime, miUptime);

        mi++;
        if (mi>59) {
            mi=0;
            hh++;
        }
        if (hh>23) {
            hh=0;
        }
        sprintf(localTime, " %02d:%02d ", hh, mi);
        
        // Update the Screensaver to display the new time
        screensaver.update(localTime);
    }

    // NTP Time update interval: 1000 * 60 * 10 = 10 Min
    if(millis() > lastUpdate + 1000 * 60 * 10){
        ntpGetTime();
    }

    // WLAN Status check and reconnect
    if(WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_STA){
        Serial.println(F("reconnect"));
        quickswitch.displayError(8);
        wlanReconnectCount++;
        quickswitch.startWLAN();
    }
}

void sendStatusToQuickSwitch(){
    lastAction = millis();
    if((millis() - alteZeit) > ENTPRELL_ZEIT) {
        quickswitch.switchDevice = 1;
        alteZeit = millis();
    }
}


void setUpAccesspoint(){
    noInterrupts();
    quickswitch.displayError(5);
    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.softAP("nodeSwitch");
    Serial.println(F("Keine Verbindung zum WLAN möglich, starte Accesspoint"));
}
/***********************

Ziel:
    jede veränderung wahrnehmbar
        um: Prozente          => update LEDs
            Farben            => update LEDs
            erste Bewegung schaltet Display ein
    dabei intervalle zum verändern der Geräte,
            => update Display

************************/
void drehencoder(int newPosition){
    if(newPosition == oldPosition || alertActive > 0){
        return;
    }

    // if(newPosition > oldPositionTick || newPosition < oldPositionTick){
    if(newPosition != oldPositionTick){
        if(screensaver.hide(quickswitch.bgColor)){
            quickswitch.clearLCD();
        }
        quickswitch.updateLCD(true);
        lastAction = millis();
        oldPositionTick = newPosition;
    }
    
    // 0 = on/off 1=percent 2=color 3=variable 4=relative
    switch(quickswitch.deviceMode){
        case 1:  //SetPercent
        case 4:{ //SetRelative
            if(newPosition >= oldPosition + 10){
                if( quickswitch.deviceList[quickswitch.selected].getStatusFloat() <= 0.95 ){
                    float newStatus = quickswitch.deviceList[quickswitch.selected].getStatusFloat() + 0.05;
                    quickswitch.deviceList[quickswitch.selected].setStatus( newStatus );
                    quickswitch.updateLCD(false);
                }
                oldPosition = newPosition;
            }
            if(newPosition <= oldPosition - 10){
                if( quickswitch.deviceList[quickswitch.selected].getStatusFloat() >= 0.05 ){
                    float newStatus = quickswitch.deviceList[quickswitch.selected].getStatusFloat() - 0.05;
                    quickswitch.deviceList[quickswitch.selected].setStatus( newStatus );
                    quickswitch.updateLCD(false);
                }
                oldPosition = newPosition;
            }
            break;
        }
        case 2:   //setColorDevice
            if(newPosition >= oldPosition + 5){
                if( quickswitch.deviceList[quickswitch.selected].getStatusFloat() < 1 ){
                    float newStatus = quickswitch.deviceList[quickswitch.selected].getStatusFloat() + 0.025;
                    quickswitch.deviceList[quickswitch.selected].setStatus( newStatus );
                    quickswitch.updateLCD(false);
                }else{
                    quickswitch.deviceList[quickswitch.selected].setStatus(0);
                }
                oldPosition = newPosition;
            }
            if(newPosition <= oldPosition - 5){
                if( quickswitch.deviceList[quickswitch.selected].getStatusFloat() > 0.025 ){
                    float newStatus = quickswitch.deviceList[quickswitch.selected].getStatusFloat() - 0.025;
                    quickswitch.deviceList[quickswitch.selected].setStatus( newStatus );
                    quickswitch.updateLCD(false);
                }else{
                    quickswitch.deviceList[quickswitch.selected].setStatus(1);
                }
                oldPosition = newPosition;
            }
            break;
        default: // ScrollDevices
            if (oldPosition != newPosition) {
                int diff = abs(oldPosition - newPosition) * ROTATIONSPEEDFACTOR;
                int ypos = quickswitch.getYPosition();
                if(newPosition > oldPosition){
                    ypos = ypos + diff;
                    if(ypos > 0){
                        ypos = 0;
                    }
                }
                if(newPosition < oldPosition){
                    ypos = ypos - diff;
                    if (ypos < ((quickswitch.deviceCount - 1) * LINEHIGHT) * -1) {
                        ypos = ((quickswitch.deviceCount - 1) * LINEHIGHT) * -1;
                    }
                }

                quickswitch.selected =  (abs(ypos) + (LINEHIGHT / 2)) / LINEHIGHT;
                quickswitch.setYPosition(ypos);
                quickswitch.updateLCD(false);
                oldPosition = newPosition;
            }
            break;
    }
}

int getIndexById(const char* id){
    for(int u = 0; u < quickswitch.deviceCount; u++){
        if(strcmp (quickswitch.deviceList[u].getId(), id) == 0){
            return u;
        }
    }
    return -1;
}


void setupWebserver(){

    server.on("/index.css", [](){
        Serial.println(F("[webserver] GET /index.css"));
        server.sendHeader("Cache-Control"," max-age=365");
        server.send(200, "text/css",
            "body,html{background:#41464d;margin:0;font:1em arial}h1{position:sticky;top:0;padding:0 10px;margin:0;font-weight:lighter;border-bottom:solid #ccc 1px;overflow:hidden;background-color:#fff}h1 div{font-size:20px;float:right;padding-top:1rem}.badge,.btn,.input{border:0;color:#000;margin:.3em 0 0 0;font-size:1em;line-height:30px;text-align:center !important;text-decoration:none;display:inline-block;outline:solid 1px lightgrey;background-color:lightgrey;min-width:14em;}.btn{cursor:pointer;font-family:Sans}.btn:hover{background-color:darkgrey}.danger{color:white;border:0 solid #000;background-color:#a00;outline:solid 1px #000}.btn.danger:hover{background-color:#800}ul{list-style:none;padding:.5em 0 2em 0;margin:0}ul ul{padding:0}li{line-height:2.5em;padding:0 1em}li:hover{background-color:#eee}li li{margin-right:-1em}li li:hover{background-color:#ccc}.right{float:right;text-align:right}.left{float:left}#updateForm{margin:0;float:right}input[type='file']{display:none}input[type='submit'],.inputfile+label{cursor:pointer}#content{background-color:#fff;height:100%;margin:0 auto;overflow-y:scroll;-ms-overflow-style:none;scrollbar-width:none}#content::-webkit-scrollbar{display:none}#footer{-webkit-backface-visibility:hidden;backface-visibility:hidden;position:fixed;bottom:0;background:#fff;width:calc(100% - 20px);text-align:end;padding:0 10px}@media(min-width:50em){#content{max-width:62.5rem}#footer{max-width:calc(62.5rem - 20px)}}@media(max-width:575.98px){.btn,.badge,.input{float:unset;width:100%}li{line-height:1}h1 div{padding:0}}.input:hover{background:#fff}"
        );
    });

    server.on("/showFont", [](){
        Serial.println(F("[webserver] GET /showFont"));
        if (!server.authenticate(WWW_USER, WWW_PASSWORD)){
            return server.requestAuthentication();
        }
        int time = 5000;
        if(server.args() == 1){
            time = server.arg(0).toInt() * 1000;
        }
        String message = FPSTR(htmlHeader);
        message += "<ul><li>Ausgeführt! Jede Seite wird für " + (String)(time / 1000) + " Sekunden gezeigt.</li></ul><meta http-equiv='refresh' content='10, url=/'>";
        message += FPSTR(htmlFooter);
        server.send(200, "text/html", message);
        noInterrupts();
        digitalWrite(DISPLAY_LIGHT_POWER_PIN, HIGH);
        quickswitch.tft.loadFont("DejaVuSansCondensed28");
        quickswitch.tft.showFont(time);
        digitalWrite(DISPLAY_LIGHT_POWER_PIN, LOW);
        quickswitch.clearLCD();
        quickswitch.updateLCD(false);
        interrupts();
    });

    server.on("/wakeup", [](){
        Serial.println(F("[webserver] GET /wakeup"));
        lastAction = millis();
        server.sendHeader("Location", String("/"), true);
        server.send(302, "text/plain", "");
        // server.send(200, "text/html", "<meta http-equiv='refresh' content='1, url=/'>");
        if(alertActive >= 0){
            return;
        }
        if(screensaver.hide(quickswitch.bgColor)){
            quickswitch.clearLCD();
            quickswitch.updateLCD(true);
            quickswitch.displayWifiStrength(true);
        }
    });

    server.on("/config", HTTP_POST, []() {
        Serial.println(F("[webserver] POST /config"));
        if (!server.authenticate(WWW_USER, WWW_PASSWORD)) {
            return server.requestAuthentication();
        }
        
        if(server.args() > 0){
            int changed = 0;
            if(server.arg("QSIP") && (String)quickswitch.config.QSIP != server.arg("QSIP") ){
                server.arg("QSIP").toCharArray(quickswitch.config.QSIP, server.arg("QSIP").length() + 1);
                changed++;
            }
            if(server.arg("QSPort") && server.arg("QSPort").toInt() != quickswitch.config.QSPort){
                quickswitch.config.QSPort = server.arg("QSPort").toInt();
                changed++;
            }
            if(server.arg("arduinoID") && server.arg("arduinoID").toInt() != quickswitch.config.arduinoID){
                quickswitch.config.arduinoID = server.arg("arduinoID").toInt();
                changed++;
            }
            if(server.arg("SSID") && (String)quickswitch.config.SSID != server.arg("SSID") ){
                server.arg("SSID").toCharArray(quickswitch.config.SSID, server.arg("SSID").length() + 1);
                changed++;
            }
            if(server.arg("password") && (String)quickswitch.config.password != server.arg("password")  && server.arg("password") != "****"){
                server.arg("password").toCharArray(quickswitch.config.password, server.arg("password").length() + 1);
                changed++;
            }
            if(server.arg("wpa2") && quickswitch.config.wpa2 != server.arg("wpa2").toInt()){
                quickswitch.config.wpa2 = server.arg("wpa2").toInt();
                changed++;
            }
            if(server.arg("identity") && (String)quickswitch.config.identity != server.arg("identity") ){
                server.arg("identity").toCharArray(quickswitch.config.identity, server.arg("identity").length() + 1);
                changed++;
            }
            if(changed > 0){
                quickswitch.saveConfig(quickswitch.config);
                server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='10, url=/'><meta charset='utf-8'> </head><body>Es wurden " + (String)changed + " Änderungen durchgeführt. Es folgt ein Neustart!</body></html>");
                delay(1000);
                ESP.restart();
            }else{
                server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='5, url=/config'><meta charset='utf-8'> </head><body>Es wurden keine Einstellungen verändert. Sie werden zu den Einstellungen weitergeleitet!</body></html>");
            }
        }
    });

    server.on("/config", HTTP_GET, []() {
        Serial.println(F("[webserver] GET /config"));
        if (!server.authenticate(WWW_USER, WWW_PASSWORD)) {
            return server.requestAuthentication();
        }
        String message = FPSTR(htmlHeader);
        message += "<form action='/config' method='POST'><ul>"
                            "<li>QuickswitchIP:  <input class='input right' type='text'  name='QSIP'        value='" + (String)quickswitch.config.QSIP          + "'></li>"
                            "<li>QuickswitchPort:<input class='input right' type='text'  name='QSPort'      value='" + (String)quickswitch.config.QSPort        + "'></li>"
                            "<li>arduinoID:      <input class='input right' type='text'  name='arduinoID'   value='" + (String)quickswitch.config.arduinoID     + "'></li>"
                            "<li>WLAN-SSID:      <input class='input right' type='text'  name='SSID'        value='" + (String)quickswitch.config.SSID          + "'></li>"
                            "<li>WLAN-Passwort:  <input class='input right' type='password' name='password' value='****'></li>"
                            "<li>WPA2-Enterprise(standard = 0): <input class='input right' type='number'name='wpa2'  value='" + (String)quickswitch.config.wpa2          + "'></li>"
                            "<li>WPA2-Username:  <input class='input right' type='text'  name='identity'    value='" + (String)quickswitch.config.identity          + "'></li>"
                            "<li><a href='/' class='left btn'>Zurück</a><input class='btn right' type='submit' value='Speichern'></li>"
                        "</ul></form>";
        message += FPSTR(htmlFooter);
        server.send(200, "text/html", message);
    });

    server.on("/alert", HTTP_POST, [](){
        Serial.println(F("[webserver] POST /alert"));
        if(server.args() > 0){
            unsigned int id      = server.arg("id").toInt();
            String       type    = server.arg("messageType");
            char         title   [31];
            char         message [31];
            char         active  [10];

            server.arg("title").toCharArray(title, 32);
            server.arg("message").toCharArray(message, 32);
            server.arg("active").toCharArray(active, 10);

            // Dismiss alert
            if(alertActive == id ){
                server.send(200);
                rtttl::stop();
                alertActive = -1;
                // switch of directly
                if(lastAction + quickswitch.displayTimeout * 1000 < millis()) {
                    quickswitch.pixel.sleep(true);
                    screensaver.show();
                }else{
                    quickswitch.clearLCD();
                    quickswitch.updateLCD(false);
                    quickswitch.displayWifiStrength(true);
                }
                return;
            }

            if(strcmp(active,"true") == 0){
                uint32_t color;
                int value = 1;
                if(type == "primary"){
                    color = quickswitch.pixel.pixels.Color(0, 0, 255);
                }else if(type == "secundary"){
                    color = quickswitch.pixel.pixels.Color(55, 55, 55);
                }else if(type == "success"){
                    color = quickswitch.pixel.pixels.Color(0, 255, 0);
                }else if(type == "danger"){
                    color = quickswitch.pixel.pixels.Color(255, 0, 0);
                }else if(type == "warning"){
                    color = quickswitch.pixel.pixels.Color(255, 255, 0);
                }else if(type == "info"){
                    color = quickswitch.pixel.pixels.Color(55, 55, 255);
                }else if(type == "light"){
                    color = quickswitch.pixel.pixels.Color(255, 255, 255);
                }else if(type == "dark"){
                    value = 0;
                }
                rtttl::begin(PIEZO_PIN, quickswitch.getSong());
                screensaver.hide(quickswitch.bgColor);

                // Using setValue for wakeup is bad... more info in led.cpp
                Serial.println("switch light on");
                quickswitch.pixel.setValue(value, color);
                quickswitch.clearLCD();
                quickswitch.tft.drawString(title,160, 90, 1);
                quickswitch.tft.unloadFont();
                quickswitch.tft.drawString(message,160, 120, 4);
                quickswitch.tft.loadFont("DejaVuSansCondensed28");
                digitalWrite(DISPLAY_LIGHT_POWER_PIN, 1);
                alertActive = id;
                lastAction = millis();
            }
            server.send(200);
        }
        server.send(422);
    });

    /*
                        "<li>"
                            "Einstellungen<a href='/config' class='right btn'>Öffnen</a>"
                        "</li>"
                        "<li>"
                            "wlanReconnectCount:<input class='badge right' type='text' name='wlanReconnectCount'value='" + (String)wlanReconnectCount + "' readonly='true'>"
                        "</li>"

                        "<li>"
                            "Update "
                            "<form method='POST' id='updateForm' class='right' style='margin:0;' action='/firmware' enctype='multipart/form-data'>"
                                "<input type='file' accept='.bin' id='updateFile' name='updateFile' class='inputfile' required/>"
                                "<label for='updateFile' class='btn'>Datei wählen</label> "
                                "<input type='submit' value='Start Update' class='btn danger'>"
                            "</form>"
                        "</li>"
                    "<script type='text/javascript'>"
                        "var input=document.getElementById('updateFile'); input.addEventListener( 'change', function(e){input.nextElementSibling.innerHTML=e.target.files[0].name;}); input.addEventListener('invalid',function(e){console.log('invaild!');input.nextElementSibling.style.outline = 'dashed 1px red'});"
                    "</script>"
    */

    server.on("/", [](){
        Serial.println(F("[webserver] GET /"));
        // Serial.println(localTime);
        // sprintf(uptime, "%d Tagen %02d:%02dh",dddd, hhUptime, miUptime);
        // Serial.println(localTime);
        // Serial.println(uptime);

        String htmlContent =  FPSTR(htmlHeader);
        htmlContent +=
                        "<ul>"
                        "<li>"
                            "Alle unterstützten Schriftzeichen ausgeben<a href='/showFont' class='right btn'>Ausführen</a>"
                        "</li>"
                        "<li>"
                            "Display<a href='/wakeup' class='right btn'>Einschalten</a>"
                        "</li>"
                        "<li>"
                            "Uhrzeit<a href='/ntpGetTime' class='right btn'>Aktualisieren</a>"
                        "</li>"
                        "<li>"
                            "Einstellungen<a href='/config' class='right btn'>Öffnen</a>"
                        "</li>"
                        "<li>"
                            "wlanReconnectCount:<input class='badge right' type='text' name='wlanReconnectCount'value='" + (String)wlanReconnectCount + "' readonly='true'>"
                        "</li>"
                        "<li>"
                            "Systime:<input class='badge right' type='text' name='systime'value='" + (String)localTime + "' readonly='true'>"
                        "</li>"
                        "<li>"
                            "Letzter Reset vor:<input class='badge right' type='text' name='uptime' value='" + (String)uptime + "' readonly='true'>"
                        "</li>"
                        "<li>"
                            "Neustartursache:<input class='badge right' type='text' name='resetReason'    value='" + (String)resetReason + "' readonly='true'>"
                        "</li>"
                        "<li>"
                            "<div class='right'>"
                                "<a href='/update' class='btn danger'>Update</a> "
                                "<a href='/reset' class='btn danger'>Neustarten</a>"
                            "</div>"
                        "</li>"
                    "</ul>";
        htmlContent += FPSTR(htmlFooter);
        server.send(200, "text/html", htmlContent );
    });

    server.on("/reset", [](){
        Serial.println(F("[webserver] GET /reset"));
        if (!server.authenticate(WWW_USER, WWW_PASSWORD)){
            return server.requestAuthentication();
        }
        String message = FPSTR(htmlHeader);
        message += "<ul><li>Restarting...</li></ul><meta http-equiv='refresh' content='10, url=/'>";
        message += FPSTR(htmlFooter);
        server.send(200, "text/html", message);
        delay(2000);
        // quickswitch.pixel.forceOff();
        // digitalWrite(DISPLAY_LIGHT_POWER_PIN, 0);
        // digitalWrite(LED_PIN, 0);
        // pinMode(DISPLAY_LIGHT_POWER_PIN, INPUT);
        // pinMode(LED_PIN, INPUT);

        // uint32_t val = READ_PERI_REG(PERIPHS_IO_MUX + 2);
        // uint32_t mask = 0x4b;  // bits[0,1,3,7] asserted. Same as value to be asserted.
        // WRITE_PERI_REG(PERIPHS_IO_MUX + 2, (!mask & val) | (mask & 0x4b));
        ESP.restart();
        // ESP.reset();
    });

    server.on("/variable", HTTP_POST, [](){
        Serial.println(F("[webserver] POST /variable"));
        if(server.args() > 0){
            char id[40];
            server.arg("id").toCharArray(id, server.arg("id").length() + 1);
            int index = getIndexById(id);
            
            if(index >= 0 ){
                Serial.print(F("setVariable: "));
                Serial.print(server.arg("id"));
                Serial.print(F(": "));
                Serial.println(server.arg("staus"));
                quickswitch.deviceList[index].setStatus(server.arg("status"));
                quickswitch.sprite.fillRect(0, (LINEHIGHT * index) + LINEHIGHT + DISPLAYOFFSET - 13, WIDTH, 23, 0); // 0 for quickswitch.bgColor, everything else for the other color (TFT_WHITE)!
                quickswitch.sprite.drawString(String(quickswitch.deviceList[index].getName()) + ": " + quickswitch.deviceList[index].getStatus() + String(quickswitch.deviceList[index].getSuffix()), 6, (LINEHIGHT * index) + LINEHIGHT + DISPLAYOFFSET, 1);
                if(alertActive < 0 && screensaver.active() == false){
                    quickswitch.updateLCD(false);
                }
            }else{
                Serial.println(F("Status einer nicht notwendigen Variable empfangen"));
            }
            server.send(200);
        }
        server.send(422);
    });

    server.on("/action", HTTP_POST, [](){
        Serial.println(F("[webserver] POST /action"));
        if(server.args() > 0){
            char id[40];
            server.arg("deviceid").toCharArray(id, server.arg("deviceid").length() + 1);
            int index = getIndexById(id);
            if(index >= 0 && quickswitch.deviceList[index].getDeviceMode() != 4){
                float status = server.arg("newStatus").toFloat();
                quickswitch.deviceList[index].setStatus(status);
                if(alertActive < 0 && screensaver.active() == false){
                    quickswitch.updateLCD(false);
                }
            }
            server.send(200);
        }
        server.send(422);
    });
    
    server.on("/ntpGetTime", [](){
        Serial.println(F("[webserver] GET /ntpGetTime"));
        // server.send(200);
        server.sendHeader("Location", String("/"), true);
        server.send(302, "text/plain", "");
        ntpGetTime();
    });

    server.onNotFound([]() {
        Serial.println(F("[webserver] onNotFound"));
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";

        for (uint8_t i = 0; i < server.args(); i++) {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }

        server.send(404, "text/plain", message);
    });

    httpUpdater.setup(&server);
    server.begin();
}

// // send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
    Serial.println(F("[NTP] sending NTP packet..."));
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

void ntpGetTime(){
    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    lastUpdate = millis();
}

void udpServerLoop(){
    int cb = udp.parsePacket();
    if(cb){
        Serial.print(F("[NTP] packet received, length="));
        Serial.println(cb);
        // We've received a packet, read the data from it
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:
    
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
    
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        ss = byte(epoch  % 60);
        mi = byte((epoch  % 3600) / 60);
        hh = byte((epoch  % 86400L) / 3600) + HOUR_OFFSET;
        if(hh > 23){
            hh-=24;
        }
        sprintf(localTime," %02d:%02d ", hh, mi);
        screensaver.update(localTime);
    }
}

bool dismissAlert(){
    if(alertActive > 0 ){
        HTTPClient http;

        String url = "http://" + String(quickswitch.config.QSIP) + ":" + String(quickswitch.config.QSPort) + "/alert/" + alertActive;
        
        Serial.print(F("[HTTP] begin: "));
        if (http.begin(url)) {
            int httpCode = http.GET();
            if (httpCode == 200) {
                Serial.println(F("success"));
                lastAction = millis();
            } else {
                Serial.print(F("errorCode: "));
                Serial.println(httpCode);
                Serial.print(F("[HTTP] "));
                Serial.println(url);
            }
        }else{
            Serial.println(F("http.begin error"));
            Serial.print(F("[HTTP] Unable to connect: "));
            Serial.println(url);
        }
        return true;
    }
    return false;
}