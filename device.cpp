#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "device.h"

const char* Device::getName(){
    return name;  
}
int Device::getDeviceMode(){
    return deviceMode;  
}
const char* Device::getId(){
    return id;
}
float Device::getStatusFloat(){
    return status.toFloat();
}
String Device::getStatus(){
    return status;
}
int Device::getStatusInt(){
    return status.toInt();
}
void Device::setStatus(float newStatus){
    if(newStatus > 1){
        newStatus = 1;
    }
    if(newStatus < 0){
        newStatus = 0;
    }
    status = newStatus;
}
void Device::setStatus(String newStatus){
    status = newStatus;
}
const char* Device::getSuffix(){
    return suffix;
}
boolean Device::turn(){
    // if(lastAction + sendTimeout < millis()){
        // schalten
        HTTPClient http;
        String usedStatus = getStatus();
        if(deviceMode == 4){
            // return true;
            if(getStatusFloat() > 0.5){
                usedStatus = "0/" + (String)(abs(getStatusFloat() * 100 - 50) / 5);
            }else{
                usedStatus = "1/" + (String)(abs(getStatusFloat() * 100 - 50) / 5);
            }
        }
        const String url = "http://" + String(this->QSIP) + ":" + String(this->QSPort) + "/switch/device/" + getId() + "/" + usedStatus;
        Serial.print(F("[HTTP] begin: "));
        if (http.begin(url)) {
            int httpCode = http.GET();
            if (httpCode == 200) {
                Serial.println("success");
                // lastAction = millis();
                return true;
            } else {
                Serial.print(F("error "));
                Serial.println(httpCode);
                Serial.print(F("[HTTP] "));
                Serial.println(url);
            }
        }else{
            Serial.println(F("http.begin error"));
            Serial.print(F("[HTTP] Unable to connect to: "));
            Serial.println(url);
        }
    // }
    return false;
}

Device::Device(const char* newId, const char* newName, const char* newSuffix, int newDeviceMode, char* newQSIP, int newQSPort){
// Device::Device(const char* newId, const char* newName, const char* newSuffix, int newDeviceMode, char* newQSIP, int newQSPort, int newSendTimeout){
    strcpy(id,   newId);
    strcpy(name, newName);
    strcpy(suffix, newSuffix);
    strcpy(QSIP, newQSIP);
    deviceMode   = newDeviceMode;
    QSPort       = newQSPort;
    // sendTimeout  = newSendTimeout;
    
    Serial.print(F("\tdeviceName: \t"));
    Serial.println(newName);
    Serial.print(F("\tdeviceID:   \t"));
    Serial.println((String)id);
    Serial.print(F("\tdeviceMode: \t"));
    Serial.println(deviceMode);
    Serial.print(F("\tdeviceSuffix: \t"));
    Serial.println(newSuffix);

    String path = "http://" + (String)QSIP + ":3333";
    switch(deviceMode){
        case 3:
            path += "/getVariableStatus/" + (String)id;
            break;
        case 4:
            status = "0.5";
            Serial.println(F("\n"));
            return; // Wichtig!
        default:
            path += "/devices/" + (String)id + "/status";
            break;
    }
    HTTPClient http;
    
    http.begin(path);
    const int statusCode = http.GET();
    String response = http.getString();
    http.end();

    if(statusCode == 200){
        response.replace((String)'"', " ");
        response.trim();
        status = response;
        Serial.print(F("\tdeviceStatus: \t"));
        Serial.print(status);
    }else{
        Serial.print(path);
        Serial.print(F("QuickSwitch meldet:"));
        Serial.println(statusCode);
        Serial.println(response);
    }
    Serial.println(F("\n"));
}

Device::Device(){}
Device::~Device(){}
