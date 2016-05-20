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
 
#include "APWizard.h"
#include "www_assets.h"

String scan_network() {
  DEBUG_PRINT(F("Scanning available networks..."));

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  byte n = WiFi.scanNetworks();

  DEBUG_PRINT(F("found "));
  DEBUG_PRINT(n);
  DEBUG_PRINT(F("..."));

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  JsonArray& networks = root.createNestedArray("networks");

  for(int i=0;i<n;i++) {
    JsonObject& network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["encryption"] = WiFi.encryptionType(i);
    network["rssi"] = WiFi.RSSI(i);
    network["bssid"] = WiFi.BSSIDstr(i);
    network["channel"] = WiFi.channel(i);
    network["isHidden"] = WiFi.isHidden(i);
  }

  root["time"] = curr_utc_time();

  String retJson;
  root.printTo(retJson);

  DEBUG_PRINTLN(F("ok!"));
  return retJson;
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