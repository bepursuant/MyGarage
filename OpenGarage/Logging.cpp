#include "Logging.h"

void Logging::init(int level, long baud){
    _level = constrain(level,LOGGING_NOOUTPUT,LOGGING_VERBOSE);
    _baud = baud;
    Serial.begin(_baud);
    this->info(CR"Started logging at loglevel [%i] to serial output at %i baud"CR, level, baud);
}

void Logging::error(char* msg, ...){
    if (LOGGING_ERROR <= _level) {   
        va_list args;
        va_start(args, msg);
        print(msg,args);
    }
}


void Logging::info(char* msg, ...){
    if (LOGGING_INFO <= _level) {
        va_list args;
        va_start(args, msg);
        print(msg,args);
    }
}

void Logging::debug(char* msg, ...){
    if (LOGGING_DEBUG <= _level) {
        va_list args;
        va_start(args, msg);
        print(msg,args);
    }
}


void Logging::verbose(char* msg, ...){
    if (LOGGING_VERBOSE <= _level) {
        va_list args;
        va_start(args, msg);
        print(msg,args);
    }
}



 void Logging::print(const char *format, va_list args) {
    //
    // loop through format string
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            if (*format == '\0') break;
            if (*format == '%') {
                Serial.print(*format);
                continue;
            }
            if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				Serial.print(s);
				continue;
			}
            if( *format == 'd' || *format == 'i') {
				Serial.print(va_arg( args, int ),DEC);
				continue;
			}
            if( *format == 'x' ) {
				Serial.print(va_arg( args, int ),HEX);
				continue;
			}
            if( *format == 'X' ) {
				Serial.print("0x");
				Serial.print(va_arg( args, int ),HEX);
				continue;
			}
            if( *format == 'b' ) {
				Serial.print(va_arg( args, int ),BIN);
				continue;
			}
            if( *format == 'B' ) {
				Serial.print("0b");
				Serial.print(va_arg( args, int ),BIN);
				continue;
			}
            if( *format == 'l' ) {
				Serial.print(va_arg( args, long ),DEC);
				continue;
			}

            if( *format == 'c' ) {
				Serial.print(va_arg( args, int ));
				continue;
			}
            if( *format == 't' ) {
				if (va_arg( args, int ) == 1) {
					Serial.print("T");
				}
				else {
					Serial.print("F");				
				}
				continue;
			}
            if( *format == 'T' ) {
				if (va_arg( args, int ) == 1) {
					Serial.print("true");
				}
				else {
					Serial.print("false");				
				}
				continue;
			}

        }
        Serial.print(*format);
    }
}

