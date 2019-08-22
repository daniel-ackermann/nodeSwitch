#include <ESP8266WiFi.h>
#include <Arduino.h>


class Debug {
    private:
        WiFiServer telnetServer = WiFiServer(23);
        // declare telnet server (do NOT put in setup())
        WiFiClient serverClient;
    public:
        void handle();
        void print(int msg);
        void print(long int msg);
        void print(char msg);
        void print(String msg);

        void println(int msg);
        void println(long int msg);
        void println(char msg);
        void println(String msg);

        void setup();
};
