#include "screensaver.h"

Screensaver::Screensaver(TFT_eSPI *tft): tft(tft){}

void Screensaver::show(){
    Serial.print(F("[Screensaver] show type: "));
    Serial.println(mode);
    switch(mode){
        case 1:
            tft->fillScreen(TFT_BLACK);
            tft->unloadFont();
            if (SPIFFS.exists("/DejaVuSansCondensed60.vlw")){
                tft->loadFont("DejaVuSansCondensed60");
                tft->setTextColor(TFT_WHITE, TFT_BLACK);
                tft->drawString(displayedValue,160, 120, 1);
            }else{
                tft->setTextColor(TFT_WHITE, TFT_BLACK);
                tft->setTextSize(3);
                tft->drawString(displayedValue,160, 120, 4);
            }
            break;
        default:
            digitalWrite(DISPLAY_LIGHT_POWER_PIN, 0);
            break;
    }
    activeNow = true;
}
/**
 *  @params bgColor: Color which is used for font background
 *  return bool true if a lcd update is needed 
 */
boolean Screensaver::hide(uint32_t bgColor){
    if(!activeNow){
        return false;
    }
    Serial.print(F("[Screensaver] hide type: "));
    Serial.println(mode);
    activeNow = false;
    switch(mode){
        case 1:
            tft->setTextColor(TFT_WHITE, bgColor);
            tft->setTextSize(1);
            tft->loadFont("DejaVuSansCondensed28");
            return true;
        default:
            digitalWrite(DISPLAY_LIGHT_POWER_PIN, 1);
            return false;
    }
}
void Screensaver::update(char value[8]){
    displayedValue = value;
    if(!activeNow){
        return;
    }
    switch(mode){
        case 1:
            tft->drawString(value,160, 120, 4);
            break;
        default:
            break;
    }
}

boolean Screensaver::active(){
    return activeNow;
}

void Screensaver::setMode(byte newMode){
    mode = newMode;
}