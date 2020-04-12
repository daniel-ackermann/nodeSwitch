#include "led.h"

void Led::setup(){
    pixels.begin();
    // for(int i = 0; i <= pixels.numPixels(); i++){
    //     pixels.setPixelColor(i, pixels.Color(0,0,0));
    // }
    pixels.show();
    // pixels.setBrightness(5);
}

void Led::startAnimation(){
    for(int i = pixels.numPixels(); i >= 0; i--){
        pixels.setPixelColor(i, pixels.Color(0,150,0));
        pixels.show();
        delay(20);
    }
    for(int i = 0; i <= pixels.numPixels(); i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
        delay(20);
    }
}

void Led::powerOnAnimation(){
    for(int i = pixels.numPixels(); i >= 0; i--){
        pixels.setPixelColor(i, pixels.Color(0,150,0));
    }
    pixels.show();
    delay(500);
    for(int i = 0; i <= pixels.numPixels(); i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
    pixels.show();
}

void Led::forceOff(){
    for(int i = 0; i <= pixels.numPixels(); i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
    pixels.show();
}

void Led::updateAnimation(){
    if(lastAnimation + LED_ANIMATION_SPEED > millis() || !animate){
        return;
    }

    sleeping = false;

    // Farbe setzen
    if(displayedColor != newColor && newValue != 0){
        pixels.fill(newColor, (NUMPIXELS) - displayedValue, displayedValue);
        pixels.show();
        displayedColor = newColor;
    }

    // Animieren
    int updateLEDs = 0;
    if(displayedStartIndex != startIndex){
        int index2          = 0;
        uint32_t color2     = displayedColor;
        if(displayedStartIndex > startIndex){
            displayedStartIndex--;
            index2          = displayedStartIndex;
        }
        if(displayedStartIndex < startIndex){
            index2          = displayedStartIndex;
            color2          = pixels.Color(0,0,0);
            displayedStartIndex++;
        }
        pixels.setPixelColor((NUMPIXELS) - 1 - index2, color2);
        updateLEDs++;
    }
    if(displayedValue != newValue){
        int index       = 0;
        uint32_t color  = displayedColor;
        if(displayedValue >  newValue){
            displayedValue--;
            index = displayedValue;
            color = pixels.Color(0,0,0);
        }
        if(displayedValue < newValue){
            index = displayedValue;
            displayedValue++;
        }
        pixels.setPixelColor((NUMPIXELS) - 1 - index,  color);
        updateLEDs++;
    }
    if(updateLEDs > 0){
        pixels.show();
    }
    

    if(displayedValue == newValue && displayedColor == newColor){
        if(displayedStartIndex == startIndex){
            Serial.print(F("[LED] animation ended: "));
            Serial.println(displayedValue);
            animate = false;          
        }
    }
    lastAnimation = millis();
}

void Led::sleep(){
    animate = false;
    sleeping = true;
    for(int i = 0; i < displayedValue; i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
        delay(20);
    }
    if(displayedStartIndex == 0){
        displayedValue = 0;
    }
}

bool Led::isSleeping(){
    return sleeping;
}
void Led::setValue(float value, uint32_t color){ // float von 0,00 - 1,00 und uint32_t mit farben
    Serial.print(F("[LED] animation started: "));
    Serial.println(value);
    startIndex    = 0;
    newValue      = (int)(value * pixels.numPixels());
    newColor      = color;
    animate       = true;
}


void Led::setValue(float value){ // float von 0,00 - 1,00
    Serial.print(F("[LED] animation started: "));
    Serial.println(value);
    startIndex    = 0;
    newValue      = (int)(value * pixels.numPixels());
    newColor      = pixels.Color(STD_COLOR_RED, STD_COLOR_GREEN, STD_COLOR_BLUE);
    animate       = true;
}

void Led::setRelative(float value){
    if(value < 0){
        startIndex    = 9 - (int)(abs(value) * 0.01 * pixels.numPixels() * 0.75);
        // startIndex    = 9 - 5;
        newValue      = 9;
    }
    if(value > 0){
        startIndex    = 9;
        // newValue      = 9 + 5;
        newValue      = 9 + (int)(value * 0.01 * pixels.numPixels() * 0.75);
    }
    if(value == 0){
        if(displayedValue == 0 && displayedStartIndex != 9){
            displayedStartIndex   = 9;
            displayedValue        = 10;
            startIndex            = 9;
            newValue              = 10;
        }else{
            startIndex    = 9;
            newValue      = 9;
        }
    }
    newColor          = pixels.Color(STD_COLOR_RED, STD_COLOR_GREEN, STD_COLOR_BLUE);
    animate           = true;
}

uint32_t Led::getDisplayedColor(){
    return displayedColor;
}

void Led::setColorDevice(float y){ // float von 0,00 - 1,00
    Serial.println(F("[LED] animation started"));
    // Serial.println("setNewColor");

    int x               = y * 100;
    /* 
    * aus    0
    * rot    0,1
    * gelb   0,16
    * grün   0,32
    * türkis 0,48
    * blau   0,64
    * lila   0,80
    * rot    0,96
    */
    int r = 0;
    int g = 0;
    int b = 0;

    // Set Max and Min
    if(x > 96){
        x = x % 96;
    }
    if(x < 0){
        x = (x % 96) * -1;  
    }

    // Set colors
    if(x == 0){
        r = 0;
        g = 0;
        b = 0;
    }
    if(x <= (int)16){
        // rot bleibt, grün wird größer
        r = 255;
        g = round(x * 15.94);
    }
    if(x > 16 && x <= 32){
        g = 255;
        r = round((32 - x) * 15.94);
    }
    if(x > 32 && x <= 48){
        g = 255;
        b = round( (x - 32) * 15.94);
    }
    if(x > 48 && x <= 64){
        b = 255;
        g = round( (64 - x) * 15.94);
    }
    if(x > 64 && x <= 80){
        b = 255;
        r = round( (x - 64) * 15.94 );  
    }
    if(x > 80 && x <= 96){
        b = round( (96 - x) * 15.94);
        r = 255;
    }

    startIndex    = 0;
    newValue      = (int)pixels.numPixels();
    newColor      = pixels.Color(r, g, b);
    animate       = true;
}

int Led::calculateColor(float y){
    int x               = y * 100;
    /* 
    * aus    0
    * rot    0,1
    * gelb   0,16
    * grün   0,32
    * türkis 0,48
    * blau   0,64
    * lila   0,80
    * rot    0,96
    */
    int r = 0;
    int g = 0;
    int b = 0;

    // Set Max and Min
    if(x > 96){
        x = x % 96;
    }
    if(x < 0){
        x = (x % 96) * -1;  
    }

    // Set colors
    if(x == 0){
        r = 0;
        g = 0;
        b = 0;
    }
    if(x <= (int)16){
        // rot bleibt, grün wird größer
        r = 255;
        g = round(x * 15.94);
    }
    if(x > 16 && x <= 32){
        g = 255;
        r = round((32 - x) * 15.94);
    }
    if(x > 32 && x <= 48){
        g = 255;
        b = round( (x - 32) * 15.94);
    }
    if(x > 48 && x <= 64){
        b = 255;
        g = round( (64 - x) * 15.94);
    }
    if(x > 64 && x <= 80){
        b = 255;
        r = round( (x - 64) * 15.94 );  
    }
    if(x > 80 && x <= 96){
        b = round( (96 - x) * 15.94);
        r = 255;
    }
    return (((byte)r & 0xF8) << 8) | (((byte)g & 0xFC) << 3) | ((byte)b >> 3);
}

/*
SetValue:
    wert => anzahl der leds + standardfarbe
    finalValue = wert * LEDs
    color       = STANDARDFARBE
    animate     = true;

setNewColor
    wert => alle leds + berechnete Farbe
    newValue  = LEDs
    color       = valueToColor(wert);
    animate     = true;

updateAnimation
    variablen:
        displayedColor
        displayedValue
        newColor
        newValue
    if(newValue != oldValue){
        // Animation
        if(displayedColor != newColor){
            // Farbe wechseln!
        }
        // alles animieren
        
    }else if(newColor != oldColor){
        // Farbe wechseln bei selbem wert!
        // wie? von null zum wert? alles auf einmal?
    }
*/