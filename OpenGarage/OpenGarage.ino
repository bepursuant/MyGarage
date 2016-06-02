/* OpenGarage Firmware
 *
 * Main loop
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
#include <ArduinoJson.h>
#include "defines.h";

// compiled HTTP assets (basically just the portal)
#include "Assets/compiled.h"

// for logging all messages to (like a serial interface)
#include "libraries/Logging.h"
#define LOGLEVEL LOGGING_VERBOSE
Logging Log = Logging();

// for storing/retrieving configuration values in the file system
#include "libraries/ConfigFile.h"
ConfigFile config;

// DEPRECATE for controlling the garage door
#include "libraries/OpenGarage.h"
OpenGarage og;

// for sending email messages
#include "libraries/SMTPMailer.h"
SMTPMailer mailer;

ESP8266WebServer *server = NULL;

static byte read_count = 0;
static uint read_value = 0;
static byte door_status = 0;

static uint led_blink_ms = LED_FAST_BLINK;
static ulong restart_timeout = 0;
static byte curr_mode;


// this is one byte storing the door status histogram
// maximum 8 bits
static byte door_status_hist = 0;


static String token = String(ESP.getChipId());
// vars to store a utc timestamp or millis() for some timeconsuming operations
static ulong last_utc = 0;		// stores the last synced UTC time
static ulong last_ntp = 0;		// stores the last millis() we synced time with ntp
static ulong last_status_check = 0;	// stores the last millis() we checked the door status
static ulong last_status_change = 0;// stores the last utc the status changed

// define a few helper functions below for interacting with the 
// server object, like sending html or json response headers
// and content to the connected client, or rendering text
void server_send_html(String html) { server->send(200, "text/html", html); }

void server_send_json(String json){ server->send(200, "text/json", json); }

void server_send_json(JsonObject& root){
	String retJson;
	root.printTo(retJson);
	server->send(200, "text/json", retJson);
}

// DEPRECATE send a validation result, including the result code [and item name]
void server_send_result(byte code, const char* item = NULL) {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["result"] = code;
	if(item)
		root["item"] = item;

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

// convert a decimal byte value into ASCII (hex)
char dec2hexchar(byte dec) {
	if(dec<10)
		return '0'+dec;
	else
		return 'A'+(dec-10);
}

// grab the mac address as bytes and return it as a string
String get_mac() {
	static String hex = "";
	if(!hex.length()) {
		byte mac[6];
		WiFi.macAddress(mac);

		for(byte i=0;i<6;i++) {
			hex += dec2hexchar((mac[i]>>4)&0x0F);
			hex += dec2hexchar(mac[i]&0x0F);
			if(i!=5) hex += ":";
		}
	}
	return hex;
}

// Generate a default SSID name, used both in AP mode
// and for MDNS registration in STA mode.
String get_ap_ssid() {
	static String ap_ssid = "";
	if(!ap_ssid.length()) {

		byte mac[6];
		WiFi.macAddress(mac);
		ap_ssid = "OG-";

		for(byte i=3;i<6;i++) {
			ap_ssid += dec2hexchar((mac[i]>>4)&0x0F);
			ap_ssid += dec2hexchar(mac[i]&0x0F);
		}

	}
	return ap_ssid;
}

// get the ip as an array of bytes and return as a dotted decimal string
String get_ap_ip() {
	static String ip = "";
	IPAddress _ip = WiFi.localIP();
	ip = _ip[0];
	ip += ".";
	ip += _ip[1];
	ip += ".";
	ip += _ip[2];
	ip += ".";
	ip += _ip[3];
	return ip;
}

// authenticate the request using the cookie token that would have been sent
// after the client calls /auth with a correct devicekey
bool is_authenticated() {
	if (server->hasHeader("Cookie")){   
		String cookie = server->header("Cookie");
		Log.info("Authenticating Cookie [%s]", cookie.c_str());
		if (cookie.indexOf("OG_TOKEN=" + token) != -1) {
			return true;
		}
	}

	server_send_result(HTML_UNAUTHORIZED);
	return false;
}

// Return the current UTC time based on the millis() since the last sync
// The exact current time is derived as a unix timestamp by taking...
// [the last timestamp we synced] + [the time since the last sync]
uint curr_utc_time(){
	return last_utc + (millis() - last_ntp)/1000;
}

void on_get_index()
{
	String html = FPSTR(html_portal);
	server_send_html(html);
}

void on_get_portal() {
	String html = FPSTR(html_portal);
	server_send_html(html);
}


void on_get_controller() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["read_value"] = read_value;
	root["door_status"] = door_status;
	root["last_status_change"] = last_status_change;
	root["read_count"] = read_count;
	root["firmware_version"] = og.options[OPTION_FIRMWARE_VERSION].ival;
	root["name"] = og.options[OPTION_NAME].sval;
	root["sensor_type"] = og.options[OPTION_SENSOR_TYPE].ival;
	root["mac"] = get_mac();
	root["cid"] = ESP.getChipId();
	
	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_get_logs() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["name"] = og.options[OPTION_NAME].sval;
	root["time"] = curr_utc_time();

	JsonArray& logs = root.createNestedArray("logs");
 
	LogStruct l;
	uint curr;
	if(!og.read_log_start()) return;
	for(uint i=0;i<og.current_log_id;i++) {
		if(!og.read_log(l,i)) continue;
		//if(!l.tstamp) continue;

		JsonObject& lg = logs.createNestedObject();
		lg["tstamp"] = l.tstamp;
		lg["status"] = l.status;
		lg["read_value"] = l.value;
	}

	og.read_log_end();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_get_config() {
	String retJson;
	config.json().printTo(retJson);
	server_send_json(retJson);
}

void on_get_status(){
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& status = root.createNestedObject("status");


	status["read_value"] = read_value;
	status["door_status"] = door_status;
	status["last_status_change"] = last_status_change;
	status["read_count"] = read_count;
	status["firmware_version"] = og.options[OPTION_FIRMWARE_VERSION].ival;
	status["name"] = og.options[OPTION_NAME].sval;
	status["mac"] = get_mac();
	status["heap"] = ESP.getFreeHeap();
	status["chipId"] = ESP.getChipId();
	status["curr_utc_time"] = curr_utc_time();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_post_controller() {
	if(server->hasArg("click")) {
		og.click_relay();
		server_send_result(HTML_SUCCESS);
	}
}

void on_post_auth() {

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	if(server->hasArg("auth_devicekey") && (server->arg("auth_devicekey") == og.options[OPTION_DEVICEKEY].sval)){
		token = server->arg("auth_devicekey");
		root["result"] = "AUTH_SUCCESS";
		root["token"] = token;
	} else {
		root["result"] = "AUTH_FAILURE";
	}

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);

}

// POST /json/options - save configuration
void on_post_config() {

	// Go through the posted arguments and save them to the configuration object
	int numArgs = server->args();

	for(byte i=0; i<numArgs; i++){
		String aKey = server->argName(i);
		String aVal = server->arg(i);
		config.json()[aKey] = aVal;
	}

	// then write the changes to the FS
	config.save();
	
	server_send_result(HTML_SUCCESS);
}

void process_ui()
{
	// process button
	static ulong button_down_time = 0;
	if(og.get_button() == LOW) {
		if(!button_down_time) {
			button_down_time = millis();
		} else {
			if(millis() > button_down_time + BUTTON_RESET_TIMEOUT) {
				led_blink_ms = 0;
				og.set_led(HIGH);
			}
		}
	}
	else {
		if (button_down_time > 0) {
			ulong curr = millis();
			if(curr > button_down_time + BUTTON_RESET_TIMEOUT) {
				//og.state = OG_STATE_RESET;
			} else if(curr > button_down_time + 50) {
				og.click_relay();
			}
			button_down_time = 0;
		}
	}
	// process led
	static ulong led_toggle_timeout = 0;
	if(led_blink_ms) {
		if(millis() > led_toggle_timeout) {
			// toggle led
			og.set_led(1-og.get_led());
			led_toggle_timeout = millis() + led_blink_ms;
		}
	}  
}

byte check_door_status_hist() {
	// perform pattern matching of door status histogram
	// and return the corresponding results
	const byte allones = (1<<DOOR_STATUS_HIST_K)-1;       // 0b1111
	const byte lowones = (1<<(DOOR_STATUS_HIST_K/2))-1; // 0b0011
	const byte highones= lowones << (DOOR_STATUS_HIST_K/2); // 0b1100
	
	byte _hist = door_status_hist & allones;  // get the lowest K bits
	if(_hist == 0) return DOOR_STATUS_REMAIN_CLOSED;
	if(_hist == allones) return DOOR_STATUS_REMAIN_OPEN;
	if(_hist == lowones) return DOOR_STATUS_JUST_OPENED;
	if(_hist == highones) return DOOR_STATUS_JUST_CLOSED;

	return DOOR_STATUS_MIXED;
}


void on_sta_upload_fin() {
	// finish update and check error
	if(!Update.end(true) || Update.hasError()) {
		server_send_result(HTML_UPLOAD_FAILED);
		return;
	}
	
	server_send_result(HTML_SUCCESS);
	restart_timeout = millis() + 2000;
}

void on_sta_upload() {
	HTTPUpload& upload = server->upload();
	Log.info("Receiving file [%s]...", upload.filename.c_str());

	if(upload.status == UPLOAD_FILE_START){
		WiFiUDP::stopAll();
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace()-0x1000)&0xFFFFF000;
		if(!Update.begin(maxSketchSpace)) {
			Log.info("not enough space...nok!");
		}
		
	} else if(upload.status == UPLOAD_FILE_WRITE) {
		Log.info(".");
		if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Log.info("size mismatch...nok!");
		}
			
	} else if(upload.status == UPLOAD_FILE_END) {
		
		Log.info("ok!");
	 
	} else if(upload.status == UPLOAD_FILE_ABORTED){
		Update.end();
		Log.info("upload aborted...nok!");
	}
	delay(0); 
}


// time keeping routine - the ESP module provides a millis() function 
// for general relative timekeeping, but we also want to know the
// basic UTC time... so lets maintain the UTC time in a var 
void time_keeping() {
	// if we haven't synced yet, or if we've gone too long without syncing,
	// lets synchronise time with the NTP server that we already set up.
	if(!last_utc || millis() > last_ntp + (TIME_SYNC_TIMEOUT * 1000) ) {
		ulong gt = time(nullptr);

		if(gt){
			last_utc = gt;
			last_ntp = millis();

			int drift = gt - curr_utc_time();

			Log.info("Synchronized time using NTP. Current unix timestamp is %i (drifted %i s)."CR, gt, drift);
		} 

	}

}

// check the status of the door based on the sensor type
void check_status() {

	switch(og.options[OPTION_SENSOR_TYPE].ival){

		case OG_SENSOR_ULTRASONIC_SIDE:
		case OG_SENSOR_ULTRASONIC_CEILING:
			// check the door status using an ultrasonic distance sensor

			DEBUG_PRINT("Reading Distance...");
			// read the distance from the sensor - let og do this because
			// it will actually give us the average of three reads
			read_value = og.read_distance();
			DEBUG_PRINT("got ");
			DEBUG_PRINT(read_value);
			Log.info("cm...ok!");

			//not sure what this is really for...
			read_count = (read_count+1)%100;

			// determine based on the current reading if the door is open or closed
			door_status = (read_value > og.options[OPTION_DTH].ival) ? 0 : 1;

			// reverse logic for side mount
			if (og.options[OPTION_SENSOR_TYPE].ival == OG_SENSOR_ULTRASONIC_SIDE)    
				door_status = 1-door_status;
		
		break;

		case OG_SENSOR_MAGNETIC_CLOSED:
			// check the door status using a single magentic sensor in the closed position
			//DEBUG_PRINT("Checking open sensor...");
			
			read_value = door_status = digitalRead(PIN_CLOSED);

			//not sure what this is really for...
			read_count = (read_count+1)%100;

			//DEBUG_PRINT("got ");
			//DEBUG_PRINT(door_status);
			//Log.info("...ok!");

		break;
	}

	// tack this status onto the histogram and find out what 'just happened'
	door_status_hist = (door_status_hist<<1) | door_status;
	byte event = check_door_status_hist();

	// write log record
	if(event == DOOR_STATUS_JUST_OPENED || event == DOOR_STATUS_JUST_CLOSED) {
		last_status_change = curr_utc_time();

		LogStruct l;
		l.tstamp = last_status_change;
		l.status = door_status;
		l.value = read_value;
		og.write_log(l);


		//Log.info("Door Changed to %s", l.status);

		// module : email alerts
		//if((bool)config.json()["smtp_notify_status"]){
			mailer.send(config.json()["smtp_from"], config.json()["smtp_to"], config.json()["smtp_subject"], (char*)last_status_change);
		//}
	}
	
}

void setup()
{
	Log.init(LOGLEVEL, 115200);

	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	Log.verbose("Configured NTP time servers %s and %s", "pool.ntp.org", "time.nist.org");

	if(server) {
		Log.verbose("Server object already existed during do_setup routine and has been erased.");
		delete server;
		server = NULL;
	}

	og.begin();
	og.options_setup();


	// setup config file and pull in values for use via config.json()["item"] later
	config.init("/jsoncfg.json");

	//WiFiManager
	//Local intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;

	//wifiManager.resetSettings();

	//set config save notify callback
	//wifiManager.setSaveConfigCallback(saveConfigCallback);

	//sets timeout until configuration portal gets turned off
	//useful to make it all retry or go to sleep
	//in seconds
	//wifiManager.setTimeout(120);

	//fetches ssid and pass and tries to connect
	//if it does not connect it starts an access point with the specified name
	//here  "AutoConnectAP"
	//and goes into a blocking loop awaiting configuration
	if (!wifiManager.autoConnect(config.json()["ssid"], config.json()["pass"])) {
		Log.error("Failed to connect to AP [%s] with password [%s]", config.json()["ssid"], config.json()["pass"]);
		delay(3000);
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(5000);
	}

	if(!server) {
		Log.info("Starting server on Port [%i]...", og.options[OPTION_HTTP_PORT].ival);

		server = new ESP8266WebServer(og.options[OPTION_HTTP_PORT].ival);

		server->on("/",   on_get_index);    
		server->on("/portal", on_get_portal);
		server->on("/auth", HTTP_POST, on_post_auth);
		server->on("/update", HTTP_POST, on_sta_upload_fin, on_sta_upload);


		server->on("/json/logs", HTTP_GET, on_get_logs);
		server->on("/json/status", HTTP_GET, on_get_status);

		server->on("/json/controller", on_post_controller);
		server->on("/json/controller", HTTP_GET, on_get_controller);

		server->on("/json/config", HTTP_GET, on_get_config);
		server->on("/json/config", HTTP_POST, on_post_config);


		server->begin();
		Log.info("ok!"CR);
	}


	mailer.init(config.json()["smtp_host"], config.json()["smtp_port"], config.json()["smtp_user"], config.json()["smtp_pass"]);


}

void loop() {

	server->handleClient();

	time_keeping();

	if(millis() > last_status_check + (og.options[OPTION_RIV].ival * 1000)) {
		last_status_check = millis();
		check_status();
	}

	process_ui();
}


void saveConfigCallback() {
  Serial.println("Should save config");
}
