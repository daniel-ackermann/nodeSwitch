#include "newSerial.h"
#include "settings.h"

newSerial::newSerial(HardwareSerial& serial  = Serial):_serial(serial){
    if(LOGGING_ACTIVE){
        serial.begin(115200);
        _serial = serial;
    }
}
//*
void newSerial::print(int msg){
    if(LOGGING_ACTIVE){
        _serial.print(msg);
    }
}

void newSerial::print(long int msg){
    if(LOGGING_ACTIVE){
        _serial.print(msg);
    }
}

void newSerial::print(char msg){
    if(LOGGING_ACTIVE){
        _serial.print(msg);
    }
}

void newSerial::print(String msg){
    if(LOGGING_ACTIVE){
        _serial.print(msg);
    }
}

void newSerial::println(int msg){
    if(LOGGING_ACTIVE){
        _serial.println(msg);
    }
}

void newSerial::println(long int msg){
    if(LOGGING_ACTIVE){
        _serial.println(msg);
    }
}

void newSerial::println(char msg){
    if(LOGGING_ACTIVE){
        _serial.println(msg);
    }
}

void newSerial::println(String msg){
    if(LOGGING_ACTIVE){
        _serial.println(msg);
    }
}
 //*/