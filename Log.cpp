// Log library provides for an easy method to output
// oLog messages and event information to the serial
// console. Support for writing to files upcoming

#include "Log.h"

void Log::init(int level, long baud) {
	_level = constrain(level, LOGLEVEL_NONE, LOGLEVEL_VERBOSE);
	_baud = baud;
	Serial.begin(_baud);

	Serial.print("\r\n\r\n");
	this->info("Serial logging has started. Baud=%i, Log Level=%i\r\n", baud, level);
}

void Log::error(const char* msg, ...) {
	if (LOGLEVEL_ERROR <= _level) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
	}
}


void Log::info(const char* msg, ...) {
	if (LOGLEVEL_INFO <= _level) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
	}
}

void Log::debug(const char* msg, ...) {
	if (LOGLEVEL_DEBUG <= _level) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
	}
}


void Log::verbose(const char* msg, ...) {
	if (LOGLEVEL_VERBOSE <= _level) {
		va_list args;
		va_start(args, msg);
		print(msg, args);
	}
}



void Log::print(const char *format, va_list args) {
	//
	// loop through format string
	Serial.print("[@");
	Serial.print(millis());
	Serial.print("/");
	Serial.print(ESP.getFreeHeap());
	Serial.print("K] ");
	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			if (*format == '\0') break;
			if (*format == '%') {
				Serial.print(*format);
				continue;
			}
			if (*format == 's') {
				register char *s = (char *)va_arg(args, int);
				Serial.print(s);
				continue;
			}
			if (*format == 'd' || *format == 'i') {
				Serial.print(va_arg(args, int), DEC);
				continue;
			}
			if (*format == 'x') {
				Serial.print(va_arg(args, int), HEX);
				continue;
			}
			if (*format == 'X') {
				Serial.print("0x");
				Serial.print(va_arg(args, int), HEX);
				continue;
			}
			if (*format == 'b') {
				Serial.print(va_arg(args, int), BIN);
				continue;
			}
			if (*format == 'B') {
				Serial.print("0b");
				Serial.print(va_arg(args, int), BIN);
				continue;
			}
			if (*format == 'l') {
				Serial.print(va_arg(args, long), DEC);
				continue;
			}

			if (*format == 'c') {
				Serial.print(va_arg(args, int));
				continue;
			}
			if (*format == 't') {
				if (va_arg(args, int) == 1) {
					Serial.print("T");
				}
				else {
					Serial.print("F");
				}
				continue;
			}
			if (*format == 'T') {
				if (va_arg(args, int) == 1) {
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