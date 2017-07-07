#include "MyGarage.h"	// global defines for MyGarage
#include "Assets.h"		// compiled assets for web portal
#include "Log.h"		// Pursuant library for logging messages
#include "Config.h"		// Pursuant library to store/retrieve config
#include "Mail.h"		// Pursuant library for sending email messages
#include "OpenGarage.h"	// DEPRECATE for controlling the garage door
#include "Button.h"		// library for reacting to button presses
#include "SerializeEeprom.h" // External library for saving to eeprom

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

// physical UI buttons
Button btn1 = Button(PIN_BTN1, BUTTON_PULLUP_INTERNAL);
Button btn2 = Button(PIN_BTN2, BUTTON_PULLUP_INTERNAL);
Button btnClosed = Button(PIN_CLOSED, BUTTON_PULLUP_INTERNAL);

// deprecate
static ulong last_status_change_utc;

// vars to store a utc timestamp for timekeeping
// (or millis() for operations that take time)
static ulong last_sync_utc = 0;				// last synced UTC time
static ulong last_sync_millis = 0;			// last millis() we synced time with ntp
static ulong portal_enabled_utc = 0;		// when the portal was enabled


// event handlers for WiFi STA
static WiFiEventHandler e1, e2, e3;

// Helper Methods

struct Network {
	String ssid;
	int32_t rssi;
	uint8_t security;
	uint8_t* bssid;
	int32_t channel;
	bool hidden;
};

void on_get_networks() {
	oLog.verbose("GET /networks ...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	int secure = 0;
	int unsecure = 0;
	int numNetworks = WiFi.scanNetworks();

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

String get_ap_ip(IPAddress _ip = WiFi.localIP()) {
	static String ip = "";
	ip += _ip[0];
	ip += ".";
	ip += _ip[1];
	ip += ".";
	ip += _ip[2];
	ip += ".";
	ip += _ip[3];
	return ip;
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

// Return the current UTC time based on the millis() since the last sync
// The exact current time is derived as a unix timestamp by taking...
// [the last timestamp we synced] + [the time since the last sync]
ulong get_utc_time() {
	return last_sync_utc + (millis() - last_sync_millis) / 1000;
}

void open_config_portal(int timeout = WIFI_PORTAL_TIMEOUT) {
	oLog.info("Opening WiFi Hotspot. Timeout=%is...", timeout);
	IPAddress portalIp(192, 168, 1, 1);
	IPAddress portalSubnet(255, 255, 255, 0);
	if (/*WiFi.mode(WIFI_AP_STA) && */WiFi.softAPConfig(portalIp, portalIp, portalSubnet) && WiFi.softAP(config.name, config.devicekey)) {
		portal_enabled_utc = get_utc_time();
		oLog.info("ok! SSID=%s, Pass=%s, IP=%s, Port=%i\r\n", config.name, config.devicekey, WiFi.softAPIP().toString().c_str(), config.http_port);
	}
	else {
		oLog.error("Could not open configuration portal as soft ap...nok!\r\n");
	}
}

void close_config_portal() {
	oLog.info("Closing WiFi HotSpot...");
	WiFi.softAPdisconnect();
	//WiFi.mode(WIFI_STA);
	portal_enabled_utc = 0;
	oLog.info("ok!\r\n");
}

void factory_reset() {
	oLog.info("Resetting to factory defaults...");

	// clear the log files
	oLog.info("formatting file system...");
	SPIFFS.format();

	oLog.info("erasing eeprom...");
	// no eeprom clear method!

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

void on_get_controller() {
	oLog.verbose("GET /controller ...");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["door_status"] = btnClosed.isPressed();
	root["last_status_change"] = last_status_change_utc;
	root["firmware_version"] = FIRMWARE_VERSION;
	root["name"] = config.name;
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

bool has_nonnull_arg(String name) {
	return (server->hasArg(name)) && (server->arg(name).length() > 0);
}

void on_post_config() {
	oLog.verbose("POST /config ...");

	int i;
	String s;
	bool needsRestart = false;

	if (has_nonnull_arg("name")) {
		oLog.debug("Config value updated. Name=name, OldValue=%s, ", config.name);
		strcpy(config.name, server->arg("name").c_str());
		oLog.debug("NewValue=%s.\r\n", config.name);
		needsRestart = true;
	}

	if (has_nonnull_arg("devicekey")) {
		oLog.debug("Config value updated. Name=devicekey, OldValue=***, NewValue=***.\r\n");
		strcpy(config.devicekey, server->arg("devicekey").c_str());
		needsRestart = true;
	}

	if (has_nonnull_arg("http_port") && (i = server->arg("http_port").toInt())) {
		oLog.debug("Config value updated. Name=http_port, OldValue=%i, NewValue=%i.\r\n", config.http_port, i);
		config.http_port = i;
		needsRestart = true;
	}

	if (server->hasArg("smtp_notify_boot") && (config.smtp_notify_boot != true)) {
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

	if (has_nonnull_arg("smtp_host")) {
		oLog.debug("Config value updated. Name=smtp_host, OldValue=%s, ", config.smtp_host);
		strcpy(config.smtp_host, server->arg("smtp_host").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_host);
		needsRestart = true;
	}

	if (has_nonnull_arg("smtp_port") && (i = server->arg("smtp_port").toInt())) {
		oLog.debug("Config value updated. Name=smtp_port, OldValue=%i, NewValue=%i.\r\n", config.smtp_port, i);
		config.smtp_port = i;
		needsRestart = true;
	}

	if (has_nonnull_arg("smtp_user")) {
		oLog.debug("Config value updated. Name=smtp_user, OldValue=%s,", config.smtp_user);
		strcpy(config.smtp_user, server->arg("smtp_user").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_user);
		needsRestart = true;
	}

	if (has_nonnull_arg("smtp_pass")) {
		oLog.debug("Config value updated. Name=smtp_pass, OldValue=%s, ", config.smtp_pass);
		strcpy(config.smtp_pass, server->arg("smtp_pass").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_pass);
		needsRestart = true;
	}

	if (has_nonnull_arg("smtp_from")) {
		oLog.debug("Config value updated. Name=smtp_from, OldValue=%s, ", config.smtp_from);
		strcpy(config.smtp_from, server->arg("smtp_from").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_from);
		needsRestart = true;
	}

	if (has_nonnull_arg("smtp_to")) {
		oLog.debug("Config value updated. Name=smtp_to, OldValue=%s, ", config.smtp_to);
		strcpy(config.smtp_to, server->arg("smtp_to").c_str());
		oLog.debug("NewValue=%s.\r\n", config.smtp_to);
		needsRestart = true;
	}

	if (has_nonnull_arg("ap_ssid")) {
		oLog.debug("Config value updated. Name=ap_ssid, OldValue=%s, ", config.ap_ssid);
		strcpy(config.ap_ssid, server->arg("ap_ssid").c_str());
		oLog.debug("NewValue=%s.\r\n", config.ap_ssid);
		needsRestart = true;
	}

	if (has_nonnull_arg("ap_pass")) {
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

void on_get_status() {
	oLog.verbose("GET /status ...");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& status = root.createNestedObject("status");

	status["door_status"] = btnClosed.isPressed();
	status["last_status_change"] = last_status_change_utc;
	status["wifi_status"] = (int)WiFi.status();
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

void on_door_status_change(byte newStatus) {
	oLog.verbose("Handling door status change. NewStatus=%i...", newStatus);

	// update timestamp of status change
	last_status_change_utc = get_utc_time();

	// write the event to the event log
	LogStruct l;
	l.tstamp = last_status_change_utc;
	l.status = newStatus;
	l.value = newStatus;
	og.write_log(l);

	// module : email alerts
	if (config.smtp_notify_status) {
		char* message;
		sprintf(message, "%s %s at %i", config.name, (newStatus == DOOR_STATUS_OPEN ? "OPENED" : "CLOSED"), last_status_change_utc);
		mail.send(config.smtp_from, config.smtp_to, message, message);
	}

	oLog.info("ok!\r\n");
}

void on_btn1_hold(Button& b) {

}

void on_btn1_click(Button& b) {
	open_config_portal();
}

void on_closed_press(Button &btn){
	on_door_status_change(DOOR_STATUS_CLOSED);
}

void on_closed_release(Button &btn){
	on_door_status_change(DOOR_STATUS_OPEN);
}


// Initializers
void init_log() {
	oLog.init(LOGLEVEL, 115200);
}

void init_fs() {
	oLog.verbose("INIT File System...");
	if (!SPIFFS.begin())
		oLog.error("Failed to Initialize File System...nok!\r\n");
	else
		oLog.info("ok!\r\n");
}

void init_config() {
	oLog.verbose("INIT Config...");
	if (!LoadConfig(config))
		oLog.error("Failed to Initialize Config...nok!\r\n");
	else
		oLog.verbose("ok!\r\n");
}

void init_ui() {
	oLog.verbose("INIT UI...");
	btn1.clickHandler(on_btn1_click);
	btn1.holdHandler(on_btn1_hold, BTN1_HOLDTIME);
	btnClosed.pressHandler(on_closed_press);
	btnClosed.releaseHandler(on_closed_release);
	oLog.verbose("ok!\r\n");
}

void init_ntp() {
	oLog.verbose("INIT NTP (Server1=%s, Server2=%s)...", "pool.ntp.org", "time.nist.org");
	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	oLog.verbose("ok!\r\n");
}

void init_wifi() {
	oLog.verbose("INIT WiFi. (SSID=%s, Pass=%s)...", config.ap_ssid, config.ap_pass);
	//WiFi.mode(WIFI_STA);
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

void init_mdns() {
	oLog.verbose("INIT MDNS (Hostname=%s)...", config.name);
	if (!MDNS.begin(config.name)) {
		oLog.error("Failed to initialize MDNS... nok!\r\n");
	}
	else {
		MDNS.addService("http", "tcp", config.http_port);
		oLog.verbose("ok!\r\n");
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


// Loop Tick Handlers
void tick_ui() {
	btn1.isPressed();
	btn2.isPressed();
	btnClosed.isPressed();
}

void tick_ws() {
	server->handleClient();
}

void tick_portal() {
	if (portal_enabled_utc > 0 && get_utc_time() > portal_enabled_utc + WIFI_PORTAL_TIMEOUT) {
		close_config_portal();
	}
}

// time keeping routine - the ESP module provides a millis() function 
// for general relative timekeeping, but we also want to know the
// basic UTC time... so lets maintain the UTC time in a var 
void tick_ntp() {
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


// Entry Point and Loop
void setup()
{
	// serial and file logging and begin output
	init_log();

	// user configuration from fs
	init_config();

	// file system for config and logs
	init_fs();

	// buttons on physical device for UI
	init_ui();

	// local clock and time servers
	init_ntp();

	// DEPRECATE: OpenGarage object
	og.begin();

	// WiFi Access Point
	init_wifi();

	// register with mdns, wont need this for a while
	init_mdns();

	// http servers
	init_ws();

	// configure the SMTP mailer
	init_smtp();
}

// The main processing loop for the application. While the
// device is powered this function will loop indefinitely
// and allow us to handle processing and UI functions
void loop() {
	// handle UI
	tick_ui();

	// handle webserver requests
	tick_ws();

	// hotspot portal timeout
	tick_portal();

	// maintain the internal clock and periodically sync via NTP
	tick_ntp();
}