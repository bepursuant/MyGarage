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

#ifndef _OPENGARAGE_H
#define _OPENGARAGE_H

#include <Arduino.h>
#include <FS.h>
#include "defines.h"

struct OptionStruct {
  String name;
  uint ival;
  uint max;
  String sval;
};

struct LogStruct {
  ulong tstamp; // time stamp
  uint status;  // door status
  uint dist;    // distance
};

class OpenGarage {
public:
  static OptionStruct options[];
  static byte state;
  static byte alarm;
  static void begin();
  static void options_setup();
  static void options_load();
  static void options_save();
  static void options_reset();
  static void restart() { digitalWrite(PIN_RESET, LOW); }
  static uint read_distance(); // centimeter
  static byte get_mode()   { return options[OPTION_MOD].ival; }
  static byte get_button() { return digitalRead(PIN_BUTTON); }
  static byte get_led()    { return digitalRead(PIN_LED); }
  static bool get_cloud_access_en();
  static bool get_local_access_en();
  static void set_led(byte status)   { digitalWrite(PIN_LED, status); }
  static void set_relay(byte status) { digitalWrite(PIN_RELAY, status); }
  static void click_relay(uint ms=1000) {
    digitalWrite(PIN_RELAY, HIGH);
    delay(ms);
    digitalWrite(PIN_RELAY, LOW);
  }
  static void open();
  static void close();
  static int find_option(String name);
  static void log_reset();
  static void write_log(const LogStruct& data);
  static bool read_log_start();
  static bool read_log_next(LogStruct& data);
  static bool read_log_end();
  static void play_note(uint freq);
  static void set_alarm() { alarm = options[OPTION_ALARM].ival * 10 + 1; }
  static void reset_alarm() { alarm = 0; }
private:
  static ulong echo_time;
  static ulong read_distance_once();
  static File log_file;
  static void button_handler();
  static void led_handler();
  static void play_startup_tune();
};

#endif  // _OPENGARAGE_H_