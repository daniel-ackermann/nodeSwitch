#include <Arduino.h>
#include "settings.h"


class Device {
    private: 
        char id[40];
        String status = "0";
        char name[20];
        char suffix[5];
        byte deviceMode;
        char QSIP[31];
        unsigned short int QSPort;
        // byte sendTimeout = 0;
        // unsigned int lastAction = millis();
    public:
        const char* getName();
        int         getDeviceMode();
        const char* getId();
        float       getStatusFloat();
        String      getStatus();
        int         getStatusInt();
        const char* getSuffix();
        void        setSendTimeout(int timeout);
        void        setStatus(float newStatus);
        void        setStatus(String newStatus);
        boolean     turn();
        // void        setup(const char* newId, const char* newName, int newDeviceMode, char* QSIP, int QSPort);
        // Constructor
        Device(const char* newId, const char* newName, const char* newSuffix, int newDeviceMode, char* QSIP, int QSPort);
        // Device(const char* newId, const char* newName, const char* newSuffix, int newDeviceMode, char* QSIP, int QSPort, int sendTimeout);
        Device();
};
