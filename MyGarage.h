#pragma once
#ifndef _DEFINES_H
#define _DEFINES_H

#include <Esp8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <time.h>
#include <FS.h>
#include <vector>

// For handling physical button presses with debouncing and long press events
//#include <Button.h>
 
/** Firmware version, hardware version, and maximal values */
#define FIRMWARE_VERSION    202  // Firmware version: 100 means 1.0.0

// gpio pins (avoid D3,4,and8)
#define PIN_CONFIG D2
#define PIN_RELAY  D5
#define PIN_LED    D6
#define PIN_CLOSED D7

// Config file name
#define CONFIG_FNAME    "/config.dat"

// Log file name
#define LOG_FNAME       "/log.dat"
#define LOGLEVEL LOGLEVEL_VERBOSE

#define MAX_LOG_RECORDS    100
#define WIFI_PORTAL_TIMEOUT 120

// door status histogram
// number of values (maximum is 8)
#define DOOR_STATUS_UNKNOWN  0
#define DOOR_STATUS_OPEN     1
#define DOOR_STATUS_CLOSED   2

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


#define BUTTON_CONFIG_HOLDTIME  5000  // if button is pressed for at least 5 seconds, reset
#define TIME_SYNC_TIMEOUT  3600

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int  uint;

#endif  // _DEFINES_H