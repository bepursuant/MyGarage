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

// for logging all messages to (like a serial interface)
#include "Logging.h"
#define LOGLEVEL LOGGING_VERBOSE
Logging Log = Logging();

#include "APWizard.h"
#include <ArduinoJson.h>

#include "OpenGarage.h"
OpenGarage og;

// for storing/retrieving configuration values in the file system
#include "ConfigFile.h"
ConfigFile config;

// for sending email messages
#include "SMTPMailer.h"
SMTPMailer mailer;

ESP8266WebServer *server = NULL;

static String scanned_ssids;

static byte read_count = 0;
static uint read_value = 0;
static byte door_status = 0;

static uint led_blink_ms = LED_FAST_BLINK;
static ulong restart_timeout = 0;
static byte curr_mode;


// this is one byte storing the door status histogram
// maximum 8 bits
static byte door_status_hist = 0;

static String token = "";

// time stuff
static ulong last_utc = 0;		// stores the last synced UTC time
static ulong last_ntp = 0;		// stores the last millis() we synced time with ntp
static ulong last_status_check = 0;	// stores the last millis() we checked the door status
static ulong last_status_change = 0;// stores the last utc the status changed

void do_setup();


// Define a few helper functions below for interacting with the 
// server object, like sending html or json response headers
// and content to the connected client, or rendering text
void server_send_html(String html) { server->send(200, "text/html", html); }

void server_send_json(String json){ server->send(200, "text/json", json); }

void server_send_json(JsonObject root){
	String retJson;
	root.printTo(retJson);
	server->send(200, "text/json", retJson);
}

// send a validation result, including the result code [and item name]
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

// The below are methods used to pull values from the requests GET
// or POST variables by key. Add an overload to optionally pull
// the value in as uint or String
bool get_value_by_key(const char* key, uint& val) {
	if(server->hasArg(key)) {
		val = server->arg(key).toInt();   
		return true;
	} else {
		return false;
	}
}

bool get_value_by_key(const char* key, String& val) {
	if(server->hasArg(key)) {
		val = server->arg(key);   
		return true;
	} else {
		return false;
	}
}

// convert a decimal byte value into it's char
char dec2hexchar(byte dec) {
	if(dec<10)
		return '0'+dec;
	else
		return 'A'+(dec-10);
}

// grab the mac address as a decimal and return it as a string
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

// get the ip as an array of octets and return as a dotted decimal string
String get_ip() {
	static String ip = "";
	if(!ip.length()) {
		IPAddress _ip = WiFi.localIP();
		ip = _ip[0];
		ip += ".";
		ip += _ip[1];
		ip += ".";
		ip += _ip[2];
		ip += ".";
		ip += _ip[3];
	}
	return ip;
}

// validate if the correct device key was provided for protected commands
bool verify_devicekey() {
	if(server->hasArg("devicekey") && (server->arg("devicekey") == og.options[OPTION_DEVICEKEY].sval))
		return true;
}

bool is_authenticated() {
	if (server->hasHeader("Cookie")){   
		String cookie = server->header("Cookie");
		DEBUG_PRINTLN(cookie);
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
	String html = "";
	
	if(curr_mode == OG_MODE_AP) {
		html += FPSTR(html_ap_home);
	} else {
		html += FPSTR(html_portal);
	}

	server_send_html(html);
}

void on_get_portal() {
	String html = FPSTR(html_portal);
	server_send_html(html);
}


void on_json_controller() {
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

void on_json_logs() {
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

void on_json_options() {
	OptionStruct *o = og.options;
	
	DynamicJsonBuffer jsonBuffer;
	String configJson;
	config.json().printTo(configJson);

	JsonObject& root = jsonBuffer.parseObject(configJson);

	for(byte i=0;i<NUM_OPTIONS;i++,o++) {
		if(!o->max) {
			root[o->name] = o->sval;
		} else {  // if this is a int option
			root[o->name] = o->ival;
		}
	}



	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_json_status(){
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

void on_controller() {
	if(server->hasArg("click")) {
		og.click_relay();
		server_send_result(HTML_SUCCESS);
	}
	if(server->hasArg("reboot")) {
		restart_timeout = millis() + 1000;
		og.state = OG_STATE_RESTART;
		server_send_result(HTML_SUCCESS);
	}
}

void on_auth() {

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

void on_post_options() {
	uint ival = 0;
	String sval;
	byte i;
	OptionStruct *o = og.options;

	int numArgs = server->args();
	for(i=0;i<numArgs;i++){
		String aKey = server->argName(i);
		String aVal = server->arg(i);
		Log.debug("%s:%s"CR,aKey.c_str(), aVal.c_str());
		config.json()[aKey] = aVal;
	}

	config.save();
	
	// FIRST ROUND: check option validity
	// do not save option values yet
	for(i=0;i<NUM_OPTIONS;i++,o++) {
		const char *key = o->name.c_str();
		// these options cannot be modified here
		if(i==OPTION_FIRMWARE_VERSION || i==OPTION_MOD  || i==OPTION_SSID ||
			i==OPTION_PASS || i==OPTION_DEVICEKEY)
			continue;
		
		if(o->max) {  // integer options
			if(get_value_by_key(key, ival)) {
				if(ival>o->max) {
					server_send_result(HTML_DATA_OUTOFBOUND, key);
					return;
				}
			}
		}
	}
	
	
	// Check device key change
	String nkey, ckey;
	const char* _nkey = "nkey";
	const char* _ckey = "ckey";
	
	if(get_value_by_key(_nkey, nkey)) {
		if(get_value_by_key(_ckey, ckey)) {
			if(!nkey.equals(ckey)) {
				server_send_result(HTML_MISMATCH, _ckey);
				return;
			}
		} else {
			server_send_result(HTML_DATA_MISSING, _ckey);
			return;
		}
	}
	
	// SECOND ROUND: change option values
	o = og.options;
	for(i=0;i<NUM_OPTIONS;i++,o++) {
		const char *key = o->name.c_str();
		// these options cannot be modified here
		if(i==OPTION_FIRMWARE_VERSION || i==OPTION_MOD  || i==OPTION_SSID ||
			i==OPTION_PASS || i==OPTION_DEVICEKEY)
			continue;
		
		if(o->max) {  // integer options
			if(get_value_by_key(key, ival)) {
				o->ival = ival;
			}
		} else {
			if(get_value_by_key(key, sval)) {
				o->sval = sval;
			}
		}
	}

	if(get_value_by_key(_nkey, nkey)) {
			og.options[OPTION_DEVICEKEY].sval = nkey;
	}

	og.options_save();
	server_send_result(HTML_SUCCESS);
}

void on_json_networks() {
	server_send_json(scanned_ssids);
}

void on_ap_change_config() {
	if(server->hasArg("ssid")) {
		og.options[OPTION_SSID].sval = server->arg("ssid");
		og.options[OPTION_PASS].sval = server->arg("pass");
		if(og.options[OPTION_SSID].sval.length() == 0) {
			server_send_result(HTML_DATA_MISSING, "ssid");
			return;
		}
		og.options_save();
		server_send_result(HTML_SUCCESS);
		og.state = OG_STATE_TRY_CONNECT;
	}
}

void on_ap_try_connect() {

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["ip"] = (WiFi.status() == WL_CONNECTED) ? (uint32_t)WiFi.localIP() : 0;

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);

	if(WiFi.status() == WL_CONNECTED && WiFi.localIP()) {
		og.options[OPTION_MOD].ival = OG_MODE_STA;
		og.options_save();  
		restart_timeout = millis() + 2000;
		og.state = OG_STATE_RESTART;
	}
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
				og.state = OG_STATE_RESET;
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

	if(!verify_devicekey()) {
		server_send_result(HTML_UNAUTHORIZED);
		//Update.reset();
	}

	// finish update and check error
	if(!Update.end(true) || Update.hasError()) {
		server_send_result(HTML_UPLOAD_FAILED);
		return;
	}
	
	server_send_result(HTML_SUCCESS);
	restart_timeout = millis() + 2000;
	og.state = OG_STATE_RESTART;
}

void on_sta_upload() {
	HTTPUpload& upload = server->upload();
	if(upload.status == UPLOAD_FILE_START){
		WiFiUDP::stopAll();
		DEBUG_PRINT(F("prepare to upload: "));
		DEBUG_PRINTLN(upload.filename);
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace()-0x1000)&0xFFFFF000;
		if(!Update.begin(maxSketchSpace)) {
			DEBUG_PRINTLN(F("not enough space"));
		}
		
	} else if(upload.status == UPLOAD_FILE_WRITE) {
		DEBUG_PRINT(".");
		if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			DEBUG_PRINTLN(F("size mismatch"));
		}
			
	} else if(upload.status == UPLOAD_FILE_END) {
		
		DEBUG_PRINTLN(F("upload completed"));
	 
	} else if(upload.status == UPLOAD_FILE_ABORTED){
		Update.end();
		DEBUG_PRINTLN(F("upload aborted"));
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
			DEBUG_PRINTLN("cm...ok!");

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
			//DEBUG_PRINTLN("...ok!");

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

void do_setup()
{
	Log.init(LOGLEVEL, 115200);

	Log.verbose("Configuring NTP Time Servers...");
	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	Log.verbose("ok!"CR);

	if(server) {
		Log.verbose("Server object already existed during do_setup routine and has been erased.");
		delete server;
		server = NULL;
	}

	og.begin();
	og.options_setup();

	curr_mode = og.get_mode();

	config.init("/jsoncfg.json");

	if(!server) {
		DEBUG_PRINT(F("Starting server on port "));
		DEBUG_PRINT(og.options[OPTION_HTTP_PORT].ival);
		DEBUG_PRINT("...");

		server = new ESP8266WebServer(og.options[OPTION_HTTP_PORT].ival);

		// routes for AP mode
		server->on("/",   on_get_index);    
		server->on("/json/networks", on_json_networks);
		server->on("/cc", on_ap_change_config);
		server->on("/jt", on_ap_try_connect);

		// routes for STA mode
		//server->on("/status", on_get_status);
		server->on("/controller", on_controller);
		server->on("/portal", on_get_portal);
		server->on("/auth", on_auth);
		server->on("/options", HTTP_POST, on_post_options);
		server->on("/update", HTTP_POST, on_sta_upload_fin, on_sta_upload);

		// define all of the json data providers / API urls
		server->on("/json/logs", on_json_logs);
		server->on("/json/status", on_json_status);
		server->on("/json/controller", on_json_controller);
		server->on("/json/options", on_json_options);

		server->begin();
		DEBUG_PRINTLN("ok!");
	}


	mailer.setup(config.json()["smtp_host"], config.json()["smtp_port"], config.json()["smtp_user"], config.json()["smtp_pass"]);


}

void do_loop() {
	static ulong connecting_timeout;
	
	switch(og.state) {

		case OG_STATE_INITIAL:
			scanned_ssids = scan_network();

			if(curr_mode == OG_MODE_AP) {
				// startup AP mode if we need configuration
				String ap_ssid = get_ap_ssid();
				start_network_ap(ap_ssid.c_str(), NULL);

				DEBUG_PRINT("IP Address: ");
				DEBUG_PRINTLN(WiFi.softAPIP());

				og.state = OG_STATE_CONNECTED;

			} else {
				// otherwise startup STA mode to connect to the router
				led_blink_ms = LED_SLOW_BLINK;
				start_network_sta(og.options[OPTION_SSID].sval.c_str(), og.options[OPTION_PASS].sval.c_str());
				connecting_timeout = millis() + 60000;

				og.state = OG_STATE_CONNECTING;

			}
			break;


		case OG_STATE_TRY_CONNECT:
			led_blink_ms = LED_SLOW_BLINK;
			start_network_sta_with_ap(og.options[OPTION_SSID].sval.c_str(), og.options[OPTION_PASS].sval.c_str());    
			og.state = OG_STATE_CONNECTED;
			break;
			
		case OG_STATE_CONNECTING:
			if(WiFi.status() == WL_CONNECTED) {
				// use ap ssid as mdns name
				if(MDNS.begin(get_ap_ssid().c_str())) {
					Log.info("Registered MDNS as %s"CR, get_ap_ssid().c_str());
				}

				og.state = OG_STATE_CONNECTED;
				led_blink_ms = 0;
				og.set_led(LOW);
				
				Log.info("Connected to AP [%s] as IP [%s]"CR, og.options[OPTION_SSID].sval.c_str(), get_ip().c_str());

				og.state = OG_STATE_CONNECTED;

			} else {
				delay(500);
				DEBUG_PRINT(".");
				if(millis() > connecting_timeout) {
					og.state = OG_STATE_INITIAL;
					DEBUG_PRINTLN(F("timeout"));
				}
			}
			break;
		
		case OG_STATE_CONNECTED:
			server->handleClient();
			break;
			
		case OG_STATE_RESTART:
			server->handleClient();
			
			if(millis() > restart_timeout) {
				og.state = OG_STATE_INITIAL;
				og.restart();
			}
			break;
			
		case OG_STATE_RESET:
			og.state = OG_STATE_INITIAL;
			DEBUG_PRINTLN(F("reset"));
			og.options_reset();
			og.log_reset();
			restart_timeout = millis() + 1000;
			og.state = OG_STATE_RESTART;
			break;
	}
	
	time_keeping();

	if(millis() > last_status_check + (og.options[OPTION_RIV].ival * 1000)) {
		last_status_check = millis();
		check_status();
	}

	process_ui();
}