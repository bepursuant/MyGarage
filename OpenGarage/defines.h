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

#ifndef _DEFINES_H
#define _DEFINES_H

/** Firmware version, hardware version, and maximal values */
#define OG_FIRMWARE_VERSION    100   // Firmware version: 100 means 1.0.0

/** GPIO pins */
#define PIN_RELAY  D1
#define PIN_BUTTON D2
#define PIN_TRIG   D3
#define PIN_ECHO   D4
#define PIN_LED    D5
#define PIN_RESET  D6
#define PIN_BUZZER D7

// Default device name
#define DEFAULT_NAME    "My OpenGarage"
// Default device key
#define DEFAULT_DEVICEKEY    "opendoor"
// Config file name
#define CONFIG_FNAME    "/config.dat"
// Log file name
#define LOG_FNAME       "/log.dat"

#define OG_ACCESS_MODE_LOCAL    0x00
#define OG_ACCESS_MODE_BOTH     0x01
#define OG_ACCESS_MODE_CLOUD    0x02

#define OG_MOUNT_TYPE_CEILING  0x00
#define OG_MOUNT_TYPE_SIDE     0x01

#define OG_ALARM_NONE     0x00
#define OG_ALARM_5        0x01
#define OG_ALARM_10       0x02

#define OG_MODE_AP       0xA9
#define OG_MODE_STA      0x2A

#define OG_STATE_INITIAL        0
#define OG_STATE_CONNECTING     1
#define OG_STATE_CONNECTED      2
#define OG_STATE_TRY_CONNECT    3
#define OG_STATE_RESET          9
#define OG_STATE_RESTART        10

#define BLYNK_PIN_LED         V0
#define BLYNK_PIN_RELAY       V1
#define BLYNK_PIN_LCD         V2
#define BLYNK_PIN_DIST        V3
#define BLYNK_PIN_READ_COUNT  V4

#define MAX_LOG_RECORDS    100
#define ALARM_FREQ         1000
// door status histogram
// number of values (maximum is 8)
#define DOOR_STATUS_HIST_K  4
#define DOOR_STATUS_REMAIN_CLOSED 0
#define DOOR_STATUS_REMAIN_OPEN   1
#define DOOR_STATUS_JUST_OPENED   2
#define DOOR_STATUS_JUST_CLOSED   3
#define DOOR_STATUS_MIXED         4

typedef enum {
  OPTION_FIRMWARE_VERSION = 0, // firmware version
  OPTION_ACCESS_MODE,     // accessbility
  OPTION_MOUNT_TYPE,     // mount type
  OPTION_DTH,     // distance threshold
  OPTION_RIV,     // read interval
  OPTION_ALARM,     // alarm mode
  OPTION_HTTP_PORT,     // http port
  OPTION_MOD,     // mode
  OPTION_SSID,    // wifi ssid
  OPTION_PASS,    // wifi password
  OPTION_AUTH,    // authentication token
  OPTION_DEVICEKEY,    // device key
  OPTION_NAME,    // device name
  NUM_OPTIONS     // number of options
} OG_OPTION_enum;

#define BUTTON_RESET_TIMEOUT  4000  // if button is pressed for at least 5 seconds, reset
#define LED_FAST_BLINK 100
#define LED_SLOW_BLINK 500

#define TIME_SYNC_TIMEOUT  3600

/** Serial debug functions */
#define SERIAL_DEBUG
#if defined(SERIAL_DEBUG)

  #define DEBUG_BEGIN(x)   { delay(2000); Serial.begin(x); }
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)

#else

  #define DEBUG_BEGIN(x)   {}
  #define DEBUG_PRINT(x)   {}
  #define DEBUG_PRINTLN(x) {}

#endif

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int  uint;

#endif  // _DEFINES_H
