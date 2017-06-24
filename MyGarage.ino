#include "MyGarage.h"	// global defines for MyGarage
#include "Assets.h"		// compiled assets for web portal
#include "Log.h"		// Pursuant library for logging messages
#include "Config.h"		// Pursuant library to store/retrieve config
#include "Mail.h"		// Pursuant library for sending email messages
#include "OpenGarage.h"	// DEPRECATE for controlling the garage door
#include "Button.h"		// library for reacting to button presses

Config config;
Log oLog;
Mail mail;
OpenGarage og;

// object that will handle the actual http server functions
ESP8266WebServer *server = NULL;
ESP8266HTTPUpdateServer *updater = NULL;

// physical buttons
Button btnConfig = Button(PIN_CONFIG, BUTTON_PULLUP_INTERNAL);
Button btnClosed = Button(PIN_CLOSED, BUTTON_PULLUP_INTERNAL);

// global door status
static byte door_status;
static ulong last_status_change_utc;

// vars to store a utc timestamp for timekeeping
// (or millis() for operations that take time)
static ulong last_sync_utc;				// last synced UTC time
static ulong last_sync_millis;			// last millis() we synced time with ntp

// define a few helper functions below for interacting with the 
// server object, like sending html or json response headers
// and content to the connected client, or rendering text
void server_send_html(String html) {
	server->send(200, "text/html", html);
}

void server_send_json(String json) {
	server->send(200, "text/json", json);
}

void server_send_json(JsonObject& root) {
	String retJson;
	root.printTo(retJson);
	server->send(200, "text/json", retJson);
}

// DEPRECATE send a validation result, including the result code [and item name]
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

// convert a decimal byte value into ASCII (hex)
char dec2hexchar(byte dec) {
	if (dec<10)
		return '0' + dec;
	else
		return 'A' + (dec - 10);
}

// check if a string is actually a number in disguise
bool is_int(String str)
{
	if (!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) {
		return false;
	}

	for (byte i = 1; i<str.length(); i++)
	{
		if (!isDigit(str.charAt(i))) {
			return false;
		}
	}

	return true;
}

// grab the mac address as bytes and return it as a string
String get_mac() {
	static String hex = "";
	if (!hex.length()) {
		byte mac[6];
		WiFi.macAddress(mac);

		for (byte i = 0; i<6; i++) {
			hex += dec2hexchar((mac[i] >> 4) & 0x0F);
			hex += dec2hexchar(mac[i] & 0x0F);
			if (i != 5) hex += ":";
		}
	}
	return hex;
}


// get the ip as an array of bytes and return as a dotted decimal string
String get_ap_ip() {
	static String ip = "";
	IPAddress _ip = WiFi.localIP();
	ip += _ip[0];
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

	if (server->hasHeader("Cookie")) {
		String cookie = server->header("Cookie");
		oLog.debug("Authenticating using Cookie. Cookie=%s...", cookie.c_str());
		if (cookie.indexOf("OG_TOKEN=" + config.devicekey) != -1) {
			oLog.debug("ok!");
			return true;
		}
	}

	server_send_result(HTML_UNAUTHORIZED);
	oLog.debug("token not found...nok!");

	return false;
}

// Return the current UTC time based on the millis() since the last sync
// The exact current time is derived as a unix timestamp by taking...
// [the last timestamp we synced] + [the time since the last sync]
ulong get_utc_time() {
	return last_sync_utc + (millis() - last_sync_millis) / 1000;
}

void on_get_index()
{
	oLog.verbose("GET /index ...");
	server_send_html(assets_portal);
	oLog.verbose("ok!\r\n");
}

void on_get_portal() {
	oLog.verbose("GET /portal ...");
	server_send_html(assets_portal);
	oLog.verbose("ok!\r\n");
}

void on_get_controller() {
	oLog.verbose("GET /controller ...");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["door_status"] = door_status;
	root["last_status_change"] = last_status_change_utc;
	root["firmware_version"] = FIRMWARE_VERSION;
	root["name"] = config.name;
	root["mac"] = get_mac();
	root["cid"] = ESP.getChipId();

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

	for (uint i = 0; i<og.current_log_id; i++) {
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
	// smtp_pass is read only
	conf["smtp_from"] = config.smtp_from;
	conf["smtp_to"] = config.smtp_to;
	conf["ap_ssid"] = config.ap_ssid;
	// ap_pass is readonly

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
	oLog.verbose("ok!\r\n");
}

void on_post_config() {
	oLog.verbose("POST /config ...");

	if (server->hasArg("name")) {
		config.name = server->arg("name");
	}

	if (server->hasArg("devicekey")) {
		config.devicekey = server->arg("devicekey");
	}

	if (server->hasArg("http_port")) {
		config.http_port = server->arg("http_port").toInt();
	}

	if (server->hasArg("smtp_notify_boot")) {
		config.smtp_notify_boot = true;
	}
	else {
		config.smtp_notify_boot = false;
	}

	if (server->hasArg("smtp_notify_status")) {
		config.smtp_notify_status = true;
	}
	else {
		config.smtp_notify_status = false;
	}

	if (server->hasArg("smtp_host")) {
		config.smtp_host = server->arg("smtp_host");
	}

	if (server->hasArg("smtp_port")) {
		config.smtp_port = server->arg("smtp_port").toInt();
	}

	if (server->hasArg("smtp_user")) {
		config.smtp_user = server->arg("smtp_user");
	}

	if (server->hasArg("smtp_pass")) {
		config.smtp_pass = server->arg("smtp_pass");
	}

	if (server->hasArg("smtp_from")) {
		config.smtp_from = server->arg("smtp_from");
	}

	if (server->hasArg("smtp_to")) {
		config.smtp_to = server->arg("smtp_to");
	}

	if (server->hasArg("ap_ssid")) {
		config.ap_ssid = server->arg("ap_ssid");
	}

	if (server->hasArg("ap_pass")) {
		config.ap_pass = server->arg("ap_pass");
	}

	// write to filesystem
	write(CONFIG_FNAME, config);

	server_send_result(HTML_SUCCESS);
	oLog.verbose("ok!\r\n");
}

void on_get_status() {
	oLog.verbose("GET /status ...");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& status = root.createNestedObject("status");

	status["door_status"] = door_status;
	status["last_status_change"] = last_status_change_utc;
	status["firmware_version"] = FIRMWARE_VERSION;
	status["name"] = config.name;
	status["mac"] = get_mac();
	status["heap"] = ESP.getFreeHeap();
	status["chip_id"] = ESP.getChipId();
	status["utc_time"] = get_utc_time();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);

	oLog.verbose("ok!\r\n");
}

void on_post_controller() {
	oLog.verbose("POST /controller");
	if (server->hasArg("click")) {
		og.click_relay();
		server_send_result(HTML_SUCCESS);
	}
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

/*
 * Handles the upload of a file from the configuration form, specifically
 * used to do firmware updates through the web interface.
 */
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

// time keeping routine - the ESP module provides a millis() function 
// for general relative timekeeping, but we also want to know the
// basic UTC time... so lets maintain the UTC time in a var 
void time_keeping() {
	// if we haven't synced yet, or if we've gone too long without syncing,
	// lets synchronise time with the NTP server that we already set up.
	if (!last_sync_utc || millis() > last_sync_millis + (TIME_SYNC_TIMEOUT * 1000)) {
		ulong gt = time(nullptr);

		if (gt) {
			last_sync_utc = gt;
			last_sync_millis = millis();
			int drift = gt - get_utc_time();
			oLog.info("Synchronized time using NTP. Current Unix Time=%i, Drift=%is.\r\n", gt, drift);
		}

	}
}

// periodically check the status of the door based on the sensor type
void onDoorSensorStatusChange() {
	oLog.verbose("Handling door status change. NewStatus=%i...", door_status);
		
	// update timestamp of status change
	last_status_change_utc = get_utc_time();

	// write the event to the event log
	LogStruct l;
	l.tstamp = last_status_change_utc;
	l.status = door_status;
	l.value = door_status;
	og.write_log(l);

	// module : email alerts
	if (config.smtp_notify_status) {
		String message = config.name + " " + (door_status == DOOR_STATUS_OPEN ? "OPENED" : "CLOSED") + " at " + String(last_status_change_utc);
		mail.send(config.smtp_from, config.smtp_to, "Door status changed!", message);
	}

	oLog.info("ok!\r\n");
}

/**
* Called when the physical button is pressed. When pressed we
* will reset the entire device to factory settings and reboot
**/
void configHoldHandler(Button& b) {
	oLog.error("Reset button held, resetting to factory defaults...");

	// clear the wifi configuration, delegated to the wifimanager. We must initialize a
	// new wifimanager object here as the original was already garbage collected
	oLog.error("reset wifi config...");
	//WiFiManager wifiManager;
	//wifiManager.resetSettings();

	// clear the FS which includes the config and oLog files
	oLog.error("formatting file system...");
	//SPIFFS.format();

	oLog.error("ok!\r\n");

	// restart the ESP to establish a fresh state
	oLog.info("ESP was reset and will now be rebooted...\r\n");
	//ESP.reset();
}


void configPressHandler(Button& b) {
	oLog.error("Config button pressed, enabling configuration portal...");

	if (WiFi.softAP(config.name.c_str(), config.devicekey.c_str())) {
		oLog.info("Configuration portal enabled by button press...ok!\r\n");
	}
	else {
		oLog.error("Could not open configuration portal as soft ap...nok!\r\n");
	}
}




void setup()
{
	// serial and file logging and begin output
	oLog.init(LOGLEVEL, 115200);
	oLog.info("Initializing MyGarage...\r\n");

	// file system for config and logs
	oLog.info("Mounting SPIFFS...");
	if (!SPIFFS.begin())
		oLog.info("failed to mount file system...nok!\r\n");
	else 
		oLog.info("ok!\r\n");

	// user configuration from fs
	oLog.verbose("Loading configuration...");
	read(CONFIG_FNAME, config);
	oLog.verbose("ok!\r\n");

	// relay
	oLog.verbose("Configuring relay...");
	pinMode(PIN_RELAY, OUTPUT);
	digitalWrite(PIN_RELAY, LOW);
	oLog.verbose("ok!\r\n");

	// internal factory reset button if held for 1 second
	oLog.verbose("Configuring human interfaces...");

	// config portal on press, reset on hold
	btnConfig.pressHandler(configPressHandler);
	btnConfig.holdHandler(configHoldHandler, BUTTON_CONFIG_HOLDTIME);

	// for when the sensor opens and closes
	btnClosed.pressHandler(closedPressHandler);
	btnClosed.releaseHandler(closedReleaseHandler);

	oLog.verbose("ok!\r\n");

	// local clock and time servers
	oLog.verbose("Configuring NTP time servers. Server1=%s, Server2=%s...", "pool.ntp.org", "time.nist.org");
	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	oLog.verbose("ok!\r\n");
	
	// DEPRECATE: OpenGarage object
	og.begin();

	// setup DHCP hostname, hopefully this works and will allow us to
	// access the device by its device name on a windows network. have 
	// to convert to a char array because our declaration is weird
	oLog.verbose("Configuring WiFi. Hostname=%s...", config.name.c_str());
	if (WiFi.hostname(config.name)) {
		oLog.verbose("ok!\r\n");
	}
	else {
		oLog.verbose("failed...nok!\r\n");
	}

	// initialize a WiFiManager to establish an existing WiFi connection
	// or offer a configuration portal to the user to connect to a new
	// network, will be garbage collected after connection
	//WiFiManager wifiManager;

	//set config save notify callback
	//wifiManager.setSaveConfigCallback(saveConfigCallback);

	//sets timeout until configuration portal gets turned off
	//useful to make it all retry or go to sleep (seconds)
	//wifiManager.setTimeout(WIFI_PORTAL_TIMEOUT);
	//wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);

	// fetches previously stored ssid and password and tries to connect
	// if it cannot connect it starts an access point where the user
	// can setup the WiFi AP then goes into blocking loop waiting
	/*oLog.info("Auto Connecting to WiFi Network...\r\n");
	if (!wifiManager.autoConnect(config.name.c_str(), NULL)) {
		oLog.info("failed to connect to AP, restarting in 30 ...nok!\r\n");
		delay(300000);
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(500000);
	}
	oLog.info("ok!\r\n");*/

	// register with mdns, wont need this for a while
	oLog.info("Configuring MDNS. Hostname=%s...", config.name.c_str());
	if (MDNS.begin(config.name.c_str())) {
		MDNS.addService("http", "tcp", 80);
		oLog.info("ok!\r\n");
	}
	else {
		oLog.info("MDNS registration failed... nok!\r\n");
	}

	oLog.info("Configuring HTTP and API Services. Port=%i...", config.http_port);

	// create a server to respond to client HTTP requests on the 
	// designated port. this will be used for the JSON API as
	// well as the single page JSON application portal
	if (server) {
		oLog.verbose("Server object already existed during setup routine and has been erased.\r\n");
		delete server;
		server = NULL;
	}

	server = new ESP8266WebServer(config.http_port);
	updater = new ESP8266HTTPUpdateServer();

	server->on("/", on_get_index);
	server->on("/portal", on_get_portal);
	server->on("/auth", HTTP_POST, on_post_auth);
	server->on("/json/logs", HTTP_GET, on_get_logs);
	server->on("/json/status", HTTP_GET, on_get_status);
	server->on("/json/controller", on_post_controller);
	server->on("/json/controller", HTTP_GET, on_get_controller);
	server->on("/json/config", HTTP_GET, on_get_config);
	server->on("/json/config", HTTP_POST, on_post_config);
	updater->setup(server);
	server->begin();

	oLog.info("ok!\r\n");

	// configure the SMTP mailer
	oLog.info("Configuring mailer. Host=%s:%i, User=%s", config.smtp_host.c_str(), config.smtp_port, config.smtp_user.c_str());
	mail.init(config.smtp_host, config.smtp_port, config.smtp_user, config.smtp_pass);
	oLog.info("ok!\r\n");

	// pull in last known door status so that we don't mistakenly send
	// a notification that a door event has occurred if the power was 
	// cycled to the device or if it was reset/restarted randomly
	int log_id = og.current_log_id;
	oLog.info("Reading most recent log item. LogID=%i ...", log_id);
	LogStruct current_log;
	og.read_log(current_log, log_id);
	oLog.info("got tstamp=%i, status=%i, value=%i...", current_log.tstamp, current_log.status, current_log.value);
	oLog.info("ok!\r\n");

	oLog.info("MyGarage has finished booting and is going into monitor mode.");

}

void closedPressHandler(Button &btn)
{
	door_status = DOOR_STATUS_OPEN;
	onDoorSensorStatusChange();
}

void closedReleaseHandler(Button &btn)
{
	door_status = DOOR_STATUS_CLOSED;
	onDoorSensorStatusChange();
}

// The main processing loop for the application. While the
// device is powered this function will loop infinitely
// and allow us to handle processing and UI function
void loop() {

	// handle human interfaces
	btnConfig.isPressed();
	btnClosed.isPressed();

	// allow the webserver to handle any client requests
	server->handleClient();

	handleWiFi();

	// maintain the internal clock and periodically sync via NTP
	time_keeping();

}

void handleWiFi()
{

}