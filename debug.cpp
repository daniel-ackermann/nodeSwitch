#include "debug.h"

void Debug::handle(){  // look for Client connect trial
    if (telnetServer.hasClient()) {
        if (!serverClient || !serverClient.connected()) {
            if (serverClient) {
                serverClient.stop();
                Serial.println("Telnet Client Stop");
            }
            serverClient = telnetServer.available();
            Serial.println("New Telnet client");
            serverClient.flush();  // clear input buffer, else you get strange characters 
        }
    }

    while(serverClient.available()) {  // get data from Client
        Serial.write(serverClient.read());
    }
}

void Debug::print(String msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.print(msg);
    }
}
void Debug::print(int msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.print(msg);
    }
}
void Debug::print(long int msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.print(msg);
    }
}
void Debug::print(char msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.print(msg);
    }
}
void Debug::println(String msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.println(msg);
    }
}
void Debug::println(long int msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.println(msg);
    }
}
void Debug::println(int msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.println(msg);
    }
}
void Debug::println(char msg){
    if (serverClient && serverClient.connected()) {  // send data to Client
        serverClient.println(msg);
    }
}

void Debug::setup(){
    telnetServer.begin();
    telnetServer.setNoDelay(true);
}
