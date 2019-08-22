#include <HardwareSerial.h>

class newSerial {
    private:
        Stream& _serial;
    public:
        newSerial(HardwareSerial& serial);
        // void print( uint32   msg);
        void print( int      msg);
        void print( long int msg);
        void print( char     msg);
        void print( String   msg);

        // void println( uint32   msg);
        void println( int      msg);
        void println( long int msg);
        void println( char     msg);
        void println( String   msg);
};
