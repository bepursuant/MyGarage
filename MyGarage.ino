#include "MyGarage.h"	// global defines for MyGarage
#include "Assets.h"		// compiled assets for web portal
#include "Log.h"		// Pursuant library for logging messages
#include "Config.h"		// Pursuant library to store/retrieve config
#include "Mail.h"		// Pursuant library for sending email messages
#include "OpenGarage.h"	// DEPRECATE for controlling the garage door
#include "Button.h"		// library for reacting to button presses
#include "Relay.h"		// Pursuant library for relays
#include "SerializeEeprom.h" // External library for saving to eeprom
#include "Led.h"		// Pursuant library for Led strip


// config object maintains configuration defaults and overrides in eeprom
Config config;

// for writing log messages to serial or logfiles
Log oLog;

// for sending email messages
Mail mail;

// deprecate - handles some logfile functions
OpenGarage og;

// http webservices
ESP8266WebServer *server = NULL;
ESP8266HTTPUpdateServer *updater = NULL;

// gpio inputs and outputs
CRGB leds[NUM_LEDS];

Button button1(PIN_BTN1);
Button button2(PIN_BTN2);

Button sensor1(PIN_SENSOR1);
Button sensor2(PIN_SENSOR2);

Relay relay1(PIN_RELAY1);
Relay relay2(PIN_RELAY2);


// vars to store a utc timestamp for timekeeping
// (or millis() for operations that take time)
ulong last_sync_utc = 0;			// last synced UTC time
ulong last_sync_millis = 0;			// last millis() we synced time with ntp
ulong last_status_change_utc;		// last time the door status changed
ulong portal_enabled_utc = 0;		// when the portal was enabled

// event handlers for WiFi STA
WiFiEventHandler e1, e2, e3;

// Helper Methods
String get_encryption_type(int type) {
	// read the encryption type and print out the name:
	switch (type) {
	case ENC_TYPE_WEP:
		return String("WEP");
		break;
	case ENC_TYPE_TKIP:
		return String("WPA");
		break;
	case ENC_TYPE_CCMP:
		return String("WPA2");
		break;
	case ENC_TYPE_NONE:
		return String("None");
		break;
	case ENC_TYPE_AUTO:
		return String("Auto");
		break;
	}
}

void server_send_html(String html) {
	server->send(200, "text/html", html);
}

void server_send_json(String json) {
	server->send(200, "text/json", json);
}

void server_send_result(byte code, const char* item = NULL) {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["result"] = code;
	if (item)
		root["item"] = item;

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

String get_sta_ip() {
	return WiFi.localIP().toString();
}

String get_ap_ip() {
	return WiFi.softAPIP().toString();
}

bool is_authenticated() {

	if (server->hasHeader("Cookie")) {
		String cookie = server->header("Cookie");
		String token = String("OG_TOKEN=") + String(config.devicekey);
		oLog.debug("Authenticating using Cookie. Cookie=%s...", cookie.c_str());
		if (cookie.indexOf(token) != -1) {
			oLog.debug("ok!");
			return true;
		}
	}

	server_send_result(HTML_UNAUTHORIZED);
	oLog.debug("token not found...nok!");

	return false;
}

bool hasRealArg(String name) {
	return (server->hasArg(name)) && (server->arg(name).length() > 0);
}


// Return the current UTC time based on the millis() since the last sync
// The exact current time is derived as a unix timestamp by taking...
// [the last timestamp we synced] + [the time since the last sync]
ulong get_utc_time() {
	return last_sync_utc + (millis() - last_sync_millis) / 1000;
}


// Reset the device to factory condition by erasing the logs, configuration,
// and connection information and finally rebooting.
void factory_reset() {
	oLog.info("Resetting to factory defaults...");

	// clear the log files
	oLog.info("formatting file system...");
	SPIFFS.format();

	//oLog.info("erasing eeprom...");
	// no eeprom clear method! configuration is invalidated by different version checksums instead

	oLog.info("ok!\r\n");

	// restart the ESP to establish a fresh state
	oLog.info("ESP was reset and will now be rebooted...\r\n");
	ESP.reset();
}


// WS Methods
void on_get_index()
{
	oLog.verbose("GET / ...");
	server_send_html(assets_portal);
	oLog.verbose("ok!\r\n");
}

void on_get_status() {
	oLog.verbose("GET /status ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonObject& status = root.createNestedObject("status");

	status["door_status"] = sensor1.isPressed();
	status["last_status_change"] = last_status_change_utc;
	status["wifi_status"] = (int)WiFi.status();
	status["wifi_rssi"] = (int)WiFi.RSSI();
	status["firmware_version"] = FIRMWARE_VERSION;
	status["name"] = config.name;
	status["heap"] = ESP.getFreeHeap();
	status["chip_id"] = ESP.getChipId();
	status["utc_time"] = get_utc_time();

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_get_logs() {
	oLog.verbose("GET /logs ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["name"] = config.name;
	root["time"] = get_utc_time();

	JsonArray& logs = root.createNestedArray("logs");

	LogStruct l;
	uint curr;
	if (!og.read_log_start()) return;

	for (uint i = 0; i < og.current_log_id; i++) {
		if (!og.read_log(l, i)) continue;
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

	oLog.verbose("ok!\r\n");
}

void on_get_config() {
	oLog.verbose("GET /config ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonObject& conf = root.createNestedObject("config");

	conf["name"] = config.name;
	conf["http_port"] = config.http_port;
	conf["smtp_notify_boot"] = config.smtp_notify_boot;
	conf["smtp_notify_status"] = config.smtp_notify_status;
	conf["smtp_host"] = config.smtp_host;
	conf["smtp_port"] = config.smtp_port;
	conf["smtp_user"] = config.smtp_user;
	// smtp_pass is protected
	conf["smtp_from"] = config.smtp_from;
	conf["smtp_to"] = config.smtp_to;
	conf["ap_ssid"] = config.ap_ssid;
	// ap_pass is protected

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_get_networks() {
	oLog.verbose("GET /networks ...");

	int secure = 0;
	int unsecure = 0;
	int numNetworks = WiFi.scanNetworks();

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonArray& networks = root.createNestedArray("networks");

	if (numNetworks > 0) {

		for (int i = 0; i < numNetworks; i++) {

			String net_ssid;
			int32_t net_rssi;
			uint8_t net_security;
			uint8_t* net_bssid;
			int32_t net_channel;
			bool net_hidden;

			WiFi.getNetworkInfo(i, net_ssid, net_security, net_rssi, net_bssid, net_channel, net_hidden);

			JsonObject& nw = networks.createNestedObject();
			nw["ssid"] = net_ssid;
			nw["rssi"] = net_rssi;
			nw["security"] = get_encryption_type(net_security);
			nw["bssid"] = net_bssid;
			nw["channel"] = net_channel;
			nw["hidden"] = net_hidden;

			// add a tally
			(net_security == ENC_TYPE_NONE) ? unsecure++ : secure++;
		}
	}

	root["count_secure"] = secure;
	root["count_unsecure"] = unsecure;

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}


void on_post_auth() {
	oLog.verbose("POST /auth ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	if (server->hasArg("devicekey") && (server->arg("devicekey") == config.devicekey)) {
		root["result"] = "AUTH_SUCCESS";
		root["token"] = config.devicekey;
		oLog.verbose("auth success...");
	}
	else {
		root["result"] = "AUTH_FAILURE";
		oLog.verbose("auth fail...");
	}

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_post_config() {
	oLog.verbose("POST /config ...");

	int i;
	String s;
	bool needsRestart = false;

	if (hasRealArg("name")) {
		oLog.debug("Config value updated. Name=name, OldValue=%s, ", config.name);
		strcpy(config.name, server->arg("name").c_str());
		oLog.debug("NewValue=%s.\r\n", config.name);
		needsRestart = true;
	}

	if (hasRealArg("devicekey")) {
		oLog.debug("Config value updated. Name=devicekey, OldValue=***, NewValue=***.\r\n");
		strcpy(config.devicekey, server->arg("devicekey").c_str());
		needsRestart = true;
	}

	if (hasRealArg("http_port") && (i = server->arg("http_port").toInt())) {
		oLog.debug("Config value updated. Name=http_port, OldValue=%i, NewValue=%i.\r\n", config.http_port, i);
		config.http_port = i;
		needsRestart = true;
	}

	if (server->hasArg("smtp_notify_boot")) {
		oLog.debug("Config value updated. Name=smtp_notify_boot, OldValue=false, NewValue=true.\r\n");
		config.smtp_notify_boot = true;
	}
	else {
		oLog.debug("Config value updated. Name=smtp_notify_boot, OldValue=true, NewValue=false.\r\n");
		config.smtp_notify_boot = false;
	}

	if (server->hasArg("smtp_notify_status")) {
		oLog.debug("Config value updated. Name=smtp_notify_status, OldValue=false, NewValue=true.\r\n");
		config.smtp_notify_status = true;
	}
	else {
		oLog.debug("Config value updated. Name=smtp_notify_status, OldValue=true, NewValue=false.\r\n");
		config.smtp_notify_status = false;
	}

	if (hasRealArg("smtp_host")) {
		oLog.debug("Config value updated. Name=smtp_host, OldValue=%s, ", config.smtp_host);
		strcpy(config.smtp_host, server->arg("smtp_host").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_host);
		needsRestart = true;
	}

	if (hasRealArg("smtp_port") && (i = server->arg("smtp_port").toInt())) {
		oLog.debug("Config value updated. Name=smtp_port, OldValue=%i, NewValue=%i.\r\n", config.smtp_port, i);
		config.smtp_port = i;
		needsRestart = true;
	}

	if (hasRealArg("smtp_user")) {
		oLog.debug("Config value updated. Name=smtp_user, OldValue=%s,", config.smtp_user);
		strcpy(config.smtp_user, server->arg("smtp_user").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_user);
		needsRestart = true;
	}

	if (hasRealArg("smtp_pass")) {
		oLog.debug("Config value updated. Name=smtp_pass, OldValue=%s, ", config.smtp_pass);
		strcpy(config.smtp_pass, server->arg("smtp_pass").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_pass);
		needsRestart = true;
	}

	if (hasRealArg("smtp_from")) {
		oLog.debug("Config value updated. Name=smtp_from, OldValue=%s, ", config.smtp_from);
		strcpy(config.smtp_from, server->arg("smtp_from").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_from);
		needsRestart = true;
	}

	if (hasRealArg("smtp_to")) {
		oLog.debug("Config value updated. Name=smtp_to, OldValue=%s, ", config.smtp_to);
		strcpy(config.smtp_to, server->arg("smtp_to").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_to);
		needsRestart = true;
	}

	if (hasRealArg("ap_ssid")) {
		oLog.debug("Config value updated. Name=ap_ssid, OldValue=%s, ", config.ap_ssid);
		strcpy(config.ap_ssid, server->arg("ap_ssid").c_str());
		oLog.debug("NewValue=%s.\r\n", config.ap_ssid);
		needsRestart = true;
	}

	if (hasRealArg("ap_pass")) {
		oLog.debug("Config value updated. Name=ap_pass, OldValue=%s, ", config.ap_pass);
		strcpy(config.ap_pass, server->arg("ap_pass").c_str());
		oLog.debug("NewValue=%s.\r\n", config.ap_pass);
		needsRestart = true;
	}

	// write to filesystem
	SaveConfig(config);

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonObject& response = root.createNestedObject("response");

	response["response_code"] = 0;
	response["needsRestart"] = needsRestart;

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_post_update_start() {
	HTTPUpload& upload = server->upload();
	oLog.info("Receiving file upload. Filename=%s...", upload.filename.c_str());

	if (upload.status == UPLOAD_FILE_START) {
		WiFiUDP::stopAll();
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if (!Update.begin(maxSketchSpace)) {
			oLog.info("not enough space...nok!\r\n");
		}
	}
	else if (upload.status == UPLOAD_FILE_WRITE) {
		oLog.info(".");
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			oLog.info("size mismatch...nok!\r\n");
		}

	}
	else if (upload.status == UPLOAD_FILE_END) {
		oLog.info("ok!\r\n");
	}
	else if (upload.status == UPLOAD_FILE_ABORTED) {
		Update.end();
		oLog.info("upload aborted...nok!\r\n");
	}
	yield(); //to keep wifi chip happy
}

void on_post_update_complete() {
	oLog.verbose("Post Update Complete...");

	// finish update and check error
	if (!Update.end(true) || Update.hasError()) {
		server_send_result(HTML_UPLOAD_FAILED);
		oLog.verbose("error, update cancelled... nok!\r\n");
		return;
	}

	server_send_result(HTML_SUCCESS);
	oLog.verbose("ok!\r\n");
}


void on_get_controller() {
	oLog.verbose("GET /controller ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["door_status"] = sensor1.isPressed();
	root["last_status_change"] = last_status_change_utc;
	root["firmware_version"] = FIRMWARE_VERSION;
	root["name"] = config.name;
	root["cid"] = ESP.getChipId();

	String retJson;
	root.printTo(retJson);
	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_post_controller() {
	oLog.verbose("POST /controller...");
	if (server->hasArg("click")) {
		relay1.click();
		server_send_result(HTML_SUCCESS);
	}
	oLog.verbose("ok!\r\n");
}

// Program Events
void on_ap_connected(WiFiEventStationModeConnected evt) {
	oLog.info("WiFi Connected to AP. SSID=%s, Channel=%i\r\n", evt.ssid.c_str(), evt.channel);
}

void on_ap_got_ip(WiFiEventStationModeGotIP evt) {
	oLog.info("WiFi Obtained IP via DHCP. IP=%s, Subnet=%s, Gateway=%s\r\n", evt.ip.toString().c_str(), evt.mask.toString().c_str(), evt.gw.toString().c_str());
}

void on_ap_disconnect(WiFiEventStationModeDisconnected evt) {
	oLog.info("WiFi Disconnected from AP. SSID=%s, Reason=%d\r\n", evt.ssid.c_str(), evt.reason);
}

void on_door_status_change(int sensor, int status) {
	oLog.verbose("Handling sensor status change. Sensor=%i, Status=%i...", sensor, status);

	// update timestamp of status change
	last_status_change_utc = get_utc_time();

	// write the event to the event log
	LogStruct l;
	l.tstamp = last_status_change_utc;
	l.status = status;
	l.value = sensor;
	og.write_log(l);

	// module : email alerts
	if (config.smtp_notify_status) {
		char* message;
		sprintf(message, "%s Sensor #%i : %s at %i", config.name,sensor, (status == DOOR_STATUS_OPEN ? "OPENED" : "CLOSED"), last_status_change_utc);
		mail.send(config.smtp_from, config.smtp_to, message, message);
	}

	oLog.info("ok!\r\n");
}


// Initializers
void init_log() {
	oLog.init(LOGLEVEL, 115200);
}

void init_config() {
	oLog.verbose("INIT Config...");
	if (!LoadConfig(config))
		oLog.error("Failed to Initialize Config...nok!\r\n");
	else
		oLog.verbose("ok!\r\n");
}

void init_fs() {
	oLog.verbose("INIT FileSystem...");
	if (!SPIFFS.begin())
		oLog.error("Failed to Initialize FileSystem...nok!\r\n");
	else
		oLog.info("ok!\r\n");
}

void init_wifi() {
	oLog.verbose("INIT WiFi. (SSID=%s, Pass=%s)...", config.ap_ssid, config.ap_pass);
	if (String(config.ap_ssid) != String("")) {
		WiFi.begin(config.ap_ssid, config.ap_pass);
		e1 = WiFi.onStationModeGotIP(on_ap_got_ip);
		e2 = WiFi.onStationModeDisconnected(on_ap_disconnect);
		e3 = WiFi.onStationModeConnected(on_ap_connected);
		delay(100);
		WiFi.hostname(config.name);
		oLog.verbose("ok!\r\n");
	} else {
		WiFi.disconnect();
		oLog.error("Failed to Initialize Wifi. No SSID Configured...nok!\r\n");
	}
}

void init_ntp() {
	oLog.verbose("INIT NTP (Server1=%s, Server2=%s)...", "pool.ntp.org", "time.nist.org");
	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	oLog.verbose("ok!\r\n");
}

void init_mdns() {
	oLog.verbose("INIT MDNS (Hostname=%s)...", config.name);
	if (MDNS.begin(config.name)) {
		oLog.verbose("ok...listing services...");
		MDNS.addService("http", "tcp", config.http_port);
		oLog.verbose("ok!\r\n");
	} else {
		oLog.error("Failed to initialize MDNS... nok!\r\n");
	}
}

void init_ws() {
	oLog.verbose("INIT HTTP and API Services (Port=%i)...", config.http_port);
	if (server) {
		oLog.debug("Server object already existed during setup routine and has been erased.\r\n");
		delete server;
		server = NULL;
	}

	server = new ESP8266WebServer(config.http_port);
	updater = new ESP8266HTTPUpdateServer();

	server->on("/", on_get_index);
	server->on("/auth", HTTP_POST, on_post_auth);
	server->on("/json/logs", HTTP_GET, on_get_logs);
	server->on("/json/status", HTTP_GET, on_get_status);
	server->on("/json/controller", on_post_controller);
	server->on("/json/controller", HTTP_GET, on_get_controller);
	server->on("/json/config", HTTP_GET, on_get_config);
	server->on("/json/config", HTTP_POST, on_post_config);
	server->on("/json/networks", HTTP_GET, on_get_networks);
	server->on("/fwlink", on_get_index);
	server->on("/generate_204", on_get_index);
	updater->setup(server);
	server->begin();

	oLog.verbose("ok!\r\n");
}

void init_smtp() {
	oLog.verbose("INIT SMTP Mailer (Host=%s:%i, User=%s)...", config.smtp_host, config.smtp_port, config.smtp_user);
	mail.init(config.smtp_host, config.smtp_port, config.smtp_user, config.smtp_pass);
	oLog.verbose("ok!\r\n");
}

void init_leds() {
	oLog.verbose("INIT LEDS...");

	FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, NUM_LEDS);
	FastLED.setBrightness(24);

	leds[LED1] = CRGB::Yellow;
	leds[LED2] = CRGB::Yellow;

	FastLED.show();

	oLog.verbose("ok!\r\n");
}

void init_ui() {
	oLog.verbose("INIT UI...");

	button1.clickHandler(on_btn1_click);
	button1.holdHandler(on_btn1_hold, BTN1_HOLDTIME);

	button2.clickHandler(on_btn2_click);
	button2.holdHandler(on_btn2_hold, BTN2_HOLDTIME);

	sensor1.pressHandler(on_sensor1_press);
	sensor1.releaseHandler(on_sensor1_release);

	sensor2.pressHandler(on_sensor2_press);
	sensor2.releaseHandler(on_sensor2_release);

	oLog.verbose("ok!\r\n");
}

void init_portal(int timeout = WIFI_PORTAL_TIMEOUT) {
	oLog.info("Opening WiFi Hotspot. Timeout=%is...", timeout);
	IPAddress portalIp(192, 168, 1, 1);
	IPAddress portalSubnet(255, 255, 255, 0);
	if (WiFi.softAPConfig(portalIp, portalIp, portalSubnet) && WiFi.softAP(config.name, config.devicekey)) {
		portal_enabled_utc = get_utc_time();
		oLog.info("ok! SSID=%s, Pass=%s, IP=%s, Port=%i\r\n", config.name, config.devicekey, portalIp.toString().c_str(), config.http_port);
	}
	else {
		oLog.error("Could not open configuration portal as soft ap...nok!\r\n");
	}
}

// Loop Tick Handlers

// time keeping routine - the ESP module provides a millis() function 
// for general relative timekeeping, but we also want to know the
// basic UTC time... so lets maintain the UTC time in a var 
void tick_ntp() {
	// if we haven't synced yet, or if we've gone too long without syncing,
	// synchronise time with the NTP server that we already set up
	if (!last_sync_utc || millis() > last_sync_millis + (TIME_SYNC_TIMEOUT * 1000)) {
		ulong gt = time(nullptr);

		if (gt) {
			last_sync_utc = gt;
			last_sync_millis = millis();
			int drift = gt - get_utc_time();
			oLog.info("Synchronized time using NTP. Current Unix Time=%i, Drift=%is.\r\n", gt, drift);
		}
		else {
			//oLog.error("Failed to sync time using NTP.");
		}

	}
}

void tick_ws() {
	server->handleClient();
}

void tick_leds() {
	FastLED.show();
}

void tick_ui() {
	button1.isPressed();
	button2.isPressed();
	sensor1.isPressed();
	sensor2.isPressed();
	//relay1.tick();
	//relay2.tick();
}

void tick_portal() {
	if (portal_enabled_utc > 0 && get_utc_time() > portal_enabled_utc + WIFI_PORTAL_TIMEOUT) {
		oLog.info("Closing WiFi HotSpot...");
		WiFi.softAPdisconnect();
		portal_enabled_utc = 0;
		oLog.info("ok!\r\n");
	}
}


// GPIOs
void on_btn1_click(Button& b) {
	oLog.info("Button 1 Click...");

	leds[LED1] = CRGB::OrangeRed;

	oLog.info("ok!\r\n");
}

void on_btn1_hold(Button& b) {
	oLog.info("Button 1 Hold. \r\n");
}

void on_btn2_click(Button& b) {
	oLog.info("Button 2 Click...");

	leds[LED2] = CRGB::FireBrick;

	oLog.info("ok!\r\n");
}

void on_btn2_hold(Button& b) {

	// button2 was held!
	oLog.info("Button 2 Hold. \r\n");

	if (button1.heldFor(BTN1_HOLDTIME)) {
		// both buttons were held
		oLog.info("Buttons 1 and 2 Hold. \r\n");
		init_portal();
	}

}


void on_sensor1_press(Button &btn) {
	leds[LED1] = CRGB::Green;
	on_door_status_change(1, DOOR_STATUS_CLOSED);
}

void on_sensor1_release(Button &btn) {
	leds[LED1] = CRGB::Red;
	on_door_status_change(1, DOOR_STATUS_OPEN);
}

void on_sensor2_press(Button &btn) {
	leds[LED2] = CRGB::Green;
	on_door_status_change(2, DOOR_STATUS_CLOSED);
}

void on_sensor2_release(Button &btn) {
	leds[LED2] = CRGB::Red;
	on_door_status_change(2, DOOR_STATUS_OPEN);
}



// Setup and Loop Methods
void setup()
{
	init_log();
	init_config();
	init_fs();

	init_wifi();
	init_ntp();
	init_mdns();


	init_ws();
	init_smtp();
	init_leds();
	init_ui();
	// init_portal() is saved for on-demand button press

}

void loop() {
	tick_ntp();
	tick_ws();
	//tick_smtp();
	tick_leds();
	tick_ui();
	tick_portal();
}