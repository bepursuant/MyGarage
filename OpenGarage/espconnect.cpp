/* OpenGarage Firmware
 *
 * ESPConnect functions
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
 
#include "espconnect.h"
#include "json.hpp"
#include "html_ap_home.h"
#include "html_sta_options.h"
#include "html_sta_home.h"
#include "html_sta_update.h"
#include "html_sta_logs.h"

using json = nlohmann::json;

// R is a C++ literal for raw string
const char html_mobile_header[] PROGMEM = R"(<head><meta name='viewport' content='width=device-width,initial-scale=1.0,minimum-scale=1.0,user-scalable=no'><title>OpenGarage</title><style>body{font-family:'helvetica';}</style></head>)";

const char html_jquery_header[] PROGMEM = "<head><title>OpenGarage</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' href='http://code.jquery.com/mobile/1.3.1/jquery.mobile-1.3.1.min.css' type='text/css'><script src='http://code.jquery.com/jquery-1.9.1.min.js' type='text/javascript'></script><script src='http://code.jquery.com/mobile/1.3.1/jquery.mobile-1.3.1.min.js' type='text/javascript'></script></head>";

const char html_ap_redirect[] PROGMEM = "<h3>WiFi config saved. Now switching to station mode.</h3>";

String scan_network() {
  DEBUG_PRINT(F("Scanning available networks..."));

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  byte n = WiFi.scanNetworks();

  DEBUG_PRINT(F("found "));
  DEBUG_PRINT(n);
  DEBUG_PRINT(F("..."));

  json networks;

  for(int i=0;i<n;i++) {
    networks["networks"][i]["ssid"] = WiFi.SSID(i);
    networks["networks"][i]["encryption"] = WiFi.encryptionType(i);
    networks["networks"][i]["rssi"] = WiFi.RSSI(i);
    networks["networks"][i]["bssid"] = WiFi.BSSID(i);
    networks["networks"][i]["bssidstr"] = WiFi.BSSIDstr(i);
    networks["networks"][i]["channel"] = WiFi.channel(i);
    networks["networks"][i]["isHidden"] = WiFi.isHidden(i);
    };
  }

  DEBUG_PRINTLN(F("ok!"));

  return networks.dump();
}

void start_network_ap(const char *ssid, const char *pass) {
  if(!ssid) return;
  DEBUG_PRINT(F("Starting AP mode..."));
  if(pass)
    WiFi.softAP(ssid, pass);
  else
    WiFi.softAP(ssid);
  WiFi.mode(WIFI_AP_STA); // start in AP_STA mode
  WiFi.disconnect();  // disconnect from router
  DEBUG_PRINTLN(F("ok!"));
}

void start_network_sta_with_ap(const char *ssid, const char *pass) {
  if(!ssid || !pass) return;
  DEBUG_PRINT(F("Starting STA mode with AP..."));
  WiFi.begin(ssid, pass);
  DEBUG_PRINTLN(F("ok!"));
}

void start_network_sta(const char *ssid, const char *pass) {
  if(!ssid || !pass) return;
  DEBUG_PRINT(F("Starting STA mode..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  DEBUG_PRINTLN("ok!");
}