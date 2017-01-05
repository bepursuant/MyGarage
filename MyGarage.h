#pragma once
#ifndef _DEFINES_H
#define _DEFINES_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <time.h>
#include <FS.h>
#include <vector>

// For handling physical button presses with debouncing and long press events
#include <Button.h>
 
/** Firmware version, hardware version, and maximal values */
#define FIRMWARE_VERSION    201  // Firmware version: 100 means 1.0.0

/** GPIO pins */
#define PIN_TRIG   D1
#define PIN_ECHO   D4
#define PIN_RELAY  D5
#define PIN_BUTTON D0
#define PIN_LED    D6
#define PIN_CLOSED D7
#define PIN_RESET  D8

// Config file name
#define CONFIG_FNAME    "/config.json"

// Log file name
#define LOG_FNAME       "/log.dat"
#define LOGLEVEL LOGLEVEL_VERBOSE


#define SENSORTYPE_ULTRASONIC_CEILING  0
#define SENSORTYPE_ULTRASONIC_SIDE     1
#define SENSORTYPE_MAGNETIC_CLOSED	   2


#define DEFAULT_NAME		"MyGarage"
#define DEFAULT_DEVICEKEY	"opendoor"
#define DEFAULT_DTH				50
#define DEFAULT_READ_INTERVAL	10
#define DEFAULT_SENSORTYPE SENSORTYPE_MAGNETIC_CLOSED
#define DEFAULT_HTTP_PORT		80
#define DEFAULT_SMTP_NOTIFY_BOOT	0
#define DEFAULT_SMTP_NOTIFY_STATUS	0
#define DEFAULT_SMTP_HOST	""
#define DEFAULT_SMTP_PORT	587
#define DEFAULT_SMTP_USER	""
#define DEFAULT_SMTP_PASS	""
#define DEFAULT_SMTP_FROM	""
#define DEFAULT_SMTP_TO		""

#define MAX_LOG_RECORDS    100
#define WIFI_PORTAL_TIMEOUT 120 //seconds

// door status histogram
// number of values (maximum is 8)
#define DOOR_STATUS_HIST_K        4
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

/** Serial debug functions */
#define DEBUG
#ifdef DEBUG

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