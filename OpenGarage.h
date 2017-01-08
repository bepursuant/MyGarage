/* OpenGarage Firmware
*
* OpenGarage library header file
* Mar 2016 @ OpenGarage.io
*
* This file is part of the OpenGarage library
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see
* <http://www.gnu.org/licenses/>.
*/

#ifndef _OPENGARAGE_h
#define _OPENGARAGE_h

#include <Arduino.h>
#include <FS.h>
#include "Log.h"
#include "MyGarage.h"

// to hold status logs
struct LogStruct {
	ulong tstamp; // time stamp
	uint status;  // door status
	uint value;    // read_value
};

class OpenGarage {
public:
	static uint current_log_id;

	static void begin();
	static void restart() { digitalWrite(PIN_RESET, LOW); }

	static void log_reset();
	static void write_log(const LogStruct& data);
	static bool read_log_start();
	static bool read_log(LogStruct& data, uint rec);
	static bool read_log_next(LogStruct& data);
	static bool read_log_end();

	static uint read_distance(); // centimeter
	static byte get_button() { return digitalRead(PIN_BUTTON); }
	static byte get_led() { return digitalRead(PIN_LED); }

	static void set_led(byte status) { digitalWrite(PIN_LED, status); }
	static void set_relay(byte status) { digitalWrite(PIN_RELAY, status); }
	static void click_relay(uint ms = 1000) {
		digitalWrite(PIN_RELAY, HIGH);
		delay(ms);
		digitalWrite(PIN_RELAY, LOW);
	}
	static bool open();
	static bool close();

private:

	static ulong read_distance_once();
	static File log_file;
	static void button_handler();
	static void led_handler();
};

#endif  // _OPENGARAGE_H_