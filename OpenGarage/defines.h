/* OpenGarage Firmware
 *
 * OpenGarage macro defines and hardware pin assignments
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
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <time.h>
#include <FS.h>

#ifndef _DEFINES_H
#define _DEFINES_H

/** Firmware version, hardware version, and maximal values */
#define OG_FIRMWARE_VERSION    100   // Firmware version: 100 means 1.0.0

/** GPIO pins */
#define PIN_TRIG   D1
#define PIN_ECHO   D4
#define PIN_RELAY  D5
#define PIN_BUTTON D0
#define PIN_LED    D6
#define PIN_CLOSED D7
#define PIN_RESET  D8

#define DEFAULT_NAME    "My OpenGarage"
#define DEFAULT_DEVICEKEY    "opendoor"
#define DEFAULT_DTH 50
#define DEFAULT_READ_INTERVAL 4
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_SMTP_NOTIFY_BOOT 0
#define DEFAULT_SMTP_NOTIFY_STATUS 0
#define DEFAULT_SMTP_HOST ""
#define DEFAULT_SMTP_PORT 587
#define DEFAULT_SMTP_USER ""
#define DEFAULT_SMTP_PASS ""
#define DEFAULT_SMTP_FROM ""
#define DEFAULT_SMTP_TO ""


// Config file name
#define CONFIG_FNAME    "/config.dat"
// Log file name
#define LOG_FNAME       "/log.dat"

#define OG_SENSOR_ULTRASONIC_CEILING  0
#define OG_SENSOR_ULTRASONIC_SIDE     1
#define OG_SENSOR_MAGNETIC_CLOSED	  2

#define OG_MODE_AP       0xA9
#define OG_MODE_STA      0x2A

#define OG_STATE_INITIAL        0
#define OG_STATE_CONNECTING     1
#define OG_STATE_CONNECTED      2
#define OG_STATE_TRY_CONNECT    3
#define OG_STATE_RESET          9
#define OG_STATE_RESTART        10

#define MAX_LOG_RECORDS    100
// door status histogram
// number of values (maximum is 8)
#define DOOR_STATUS_HIST_K  4
#define DOOR_STATUS_REMAIN_CLOSED 0
#define DOOR_STATUS_REMAIN_OPEN   1
#define DOOR_STATUS_JUST_OPENED   2
#define DOOR_STATUS_JUST_CLOSED   3
#define DOOR_STATUS_MIXED         4

#define HTML_OK                0x00
#define HTML_SUCCESS           0x01
#define HTML_UNAUTHORIZED      0x02
#define HTML_MISMATCH          0x03
#define HTML_DATA_MISSING      0x10
#define HTML_DATA_OUTOFBOUND   0x11
#define HTML_DATA_FORMATERROR  0x12
#define HTML_PAGE_NOT_FOUND    0x20
#define HTML_FILE_NOT_FOUND    0x21
#define HTML_NOT_PERMITTED     0x30
#define HTML_UPLOAD_FAILED     0x40
#define HTML_REDIRECT_HOME     0xFF


#define BUTTON_RESET_TIMEOUT  4000  // if button is pressed for at least 5 seconds, reset
#define LED_FAST_BLINK 100
#define LED_SLOW_BLINK 500

#define TIME_SYNC_TIMEOUT  3600
#define CONNECT_AP_TIMEOUT 30000

/** Serial debug functions */
#define SERIAL_DEBUG
#ifdef SERIAL_DEBUG

	#define DEBUG_BEGIN(x)  { delay(2000); Serial.begin(x); }
	#define DEBUG_PRINT(x) { Serial.print(x); }
	#define DEBUG_PRINTLN(x) { Serial.println(x); }

#else

	#define DEBUG_BEGIN(x)   {}
	#define DEBUG_PRINT(x)   {}
	#define DEBUG_PRINTLN(x) {}

#endif

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int  uint;

#endif  // _DEFINES_H