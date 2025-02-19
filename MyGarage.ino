#include "MyGarage.h"	// global defines for MyGarage
#include "Assets.h"		// compiled assets for web portal

// for logging all messages
#include "Log.h"
Log oLog = Log();

// for storing/retrieving config values and loading/saving to json file
#include "Config.h"
vector<ConfigStruct> defaultConfig = {
	{ "name", DEFAULT_NAME },
	{ "devicekey", DEFAULT_DEVICEKEY },
	{ "http_port", DEFAULT_HTTP_PORT },
	{ "dth", DEFAULT_DTH },
	{ "read_interval", DEFAULT_READ_INTERVAL },
	{ "sensor_type", SENSORTYPE_ULTRASONIC_CEILING },
	{ "smtp_notify_boot", DEFAULT_SMTP_NOTIFY_BOOT },
	{ "smtp_notify_status", DEFAULT_SMTP_NOTIFY_STATUS },
	{ "smtp_host", DEFAULT_SMTP_HOST },
	{ "smtp_port", DEFAULT_SMTP_PORT },
	{ "smtp_user", DEFAULT_SMTP_USER },
	{ "smtp_pass", DEFAULT_SMTP_PASS },
	{ "smtp_from", DEFAULT_SMTP_FROM },
	{ "smtp_to", DEFAULT_SMTP_TO }
};
Config config = Config();

// DEPRECATE for controlling the garage door
#include "OpenGarage.h"
OpenGarage og;

// for sending email messages
#include "Mail.h"
Mail mail;

// physical button for reset
Button bReset(PIN_BUTTON, BUTTON_PULLUP_INTERNAL);

// object that will handle the actual http server functions
ESP8266WebServer *server = NULL;

static byte read_count = 0;
static uint read_value = 0;
static byte door_status = 0;

static uint led_blink_ms = LED_FAST_BLINK;
static ulong restart_timeout = 0;

// this is one byte (8 bits) storing the door status histogram
static byte door_status_hist = 0;

// to be implemented - just a string for now, will need to be hashed later
// basically just so we don't store the plaintext devicekey in a cookie
static String token = "opendoor"; //String(config.getString("devicekey"));

// vars to store a utc timestamp for timekeeping
// (or millis() for operations that take time)
static ulong last_utc = 0;			// last synced UTC time
static ulong last_ntp = 0;			// last millis() we synced time with ntp
static ulong last_status_check = 0;	// last millis() we checked the door status
static ulong last_status_change = 0;// last utc the status changed

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
bool isInt(String str)
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

// Generate a default SSID name, used both in AP mode
// and for MDNS registration in STA mode.
String get_ap_ssid() {
	static String ap_ssid = "";
	if (!ap_ssid.length()) {

		byte mac[6];
		WiFi.macAddress(mac);
		ap_ssid = "OG-";

		for (byte i = 3; i<6; i++) {
			ap_ssid += dec2hexchar((mac[i] >> 4) & 0x0F);
			ap_ssid += dec2hexchar(mac[i] & 0x0F);
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
	if (server->hasHeader("Cookie")) {
		String cookie = server->header("Cookie");
		oLog.info("Authenticating using Cookie. Cookie=%s...", cookie.c_str());
		if (cookie.indexOf("OG_TOKEN=" + token) != -1) {
			oLog.info("ok!");
			return true;
		}
	}

	server_send_result(HTML_UNAUTHORIZED);
	oLog.info("token not found...nok!");
	return false;
}

// Return the current UTC time based on the millis() since the last sync
// The exact current time is derived as a unix timestamp by taking...
// [the last timestamp we synced] + [the time since the last sync]
uint curr_utc_time() {
	return last_utc + (millis() - last_ntp) / 1000;
}

void on_get_index()
{
	String html = FPSTR(assets_portal);
	server_send_html(html);
}

void on_get_portal() {
	String html = FPSTR(assets_portal);
	server_send_html(html);
}

void on_get_controller() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["read_value"] = (int)read_value;
	root["door_status"] = (int)door_status;
	root["last_status_change"] = (int)last_status_change;
	root["read_count"] = (int)read_count;
	root["firmware_version"] = (int)FIRMWARE_VERSION;
	root["name"] = config.getString("name");
	root["sensor_type"] = config.getInt("sensor_type");
	root["mac"] = get_mac();
	root["cid"] = (int)ESP.getChipId();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_get_logs() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["name"] = config.getString("name");
	root["time"] = (int)curr_utc_time();

	JsonArray& logs = root.createNestedArray("logs");

	LogStruct l;
	uint curr;
	if (!og.read_log_start()) return;
	for (uint i = 0; i<og.current_log_id; i++) {
		if (!og.read_log(l, i)) continue;
		//if(!l.tstamp) continue;

		JsonObject& lg = logs.createNestedObject();
		lg["tstamp"] = (int)l.tstamp;
		lg["status"] = (int)l.status;
		lg["read_value"] = (int)l.value;
	}

	og.read_log_end();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_get_config() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& conf = root.createNestedObject("config");

	conf["name"] = config.getString("name");
	conf["devicekey"] = config.getString("devicekey");
	conf["http_port"] = config.getInt("http_port");
	conf["dth"] = config.getInt("dth");
	conf["read_interval"] = config.getInt("read_interval");
	conf["sensor_type"] = config.getInt("sensor_type");
	conf["smtp_notify_boot"] = config.getString("smtp_notify_boot");
	conf["smtp_notify_status"] = config.getString("smtp_notify_status");
	conf["smtp_host"] = config.getString("smtp_host");
	conf["smtp_port"] = config.getInt("smtp_port");
	conf["smtp_user"] = config.getString("smtp_user");
	conf["smtp_pass"] = config.getString("smtp_pass");
	conf["smtp_from"] = config.getString("smtp_from");
	conf["smtp_to"] = config.getString("smtp_to");

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_post_config() {
	// Go through the posted arguments and attempt
	// to save them to the configuration object
	int numArgs = server->args();
	for (int i = 0; i < numArgs; i++) {

		//find the key, string value and interpolate the ival
		//by casting to an int. we store the value in both
		//str and int to provide easier x-compatibility
		String key = server->argName(i);
		String sval = String(server->arg(i));

		config.set(key, sval);
	}

	config.saveJsonFile(CONFIG_FNAME);

	server_send_result(HTML_SUCCESS);
}

void on_get_status() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	JsonObject& status = root.createNestedObject("status");

	status["read_value"] = (int)read_value;
	status["door_status"] = (int)door_status;
	status["last_status_change"] = (int)last_status_change;
	status["read_count"] = read_count;
	status["firmware_version"] = (int)FIRMWARE_VERSION;
	status["name"] = config.getString("name");
	status["mac"] = get_mac();
	status["heap"] = (int)ESP.getFreeHeap();
	status["chipId"] = (int)ESP.getChipId();
	status["curr_utc_time"] = (int)curr_utc_time();

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);
}

void on_post_controller() {
	if (server->hasArg("click")) {
		og.click_relay();
		server_send_result(HTML_SUCCESS);
	}
}

void on_post_auth() {

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	if (server->hasArg("auth_devicekey") && (server->arg("auth_devicekey") == config.getString("devicekey"))) {
		token = server->arg("auth_devicekey");
		root["result"] = "AUTH_SUCCESS";
		root["token"] = token;
	}
	else {
		root["result"] = "AUTH_FAILURE";
	}

	String retJson;
	root.printTo(retJson);

	server_send_json(retJson);

}

void process_ui()
{
	// process button
	static ulong button_down_time = 0;
	if (og.get_button() == LOW) {
		if (!button_down_time) {
			button_down_time = millis();
		}
		else {
			if (millis() > button_down_time + BUTTON_RESET_TIMEOUT) {
				led_blink_ms = 0;
				og.set_led(HIGH);
			}
		}
	}
	else {
		if (button_down_time > 0) {
			ulong curr = millis();
			if (curr > button_down_time + BUTTON_RESET_TIMEOUT) {
				//og.state = OG_STATE_RESET;
			}
			else if (curr > button_down_time + 50) {
				og.click_relay();
			}
			button_down_time = 0;
		}
	}

	// process led
	static ulong led_toggle_timeout = 0;
	if (led_blink_ms) {
		if (millis() > led_toggle_timeout) {
			// toggle led
			og.set_led(1 - og.get_led());
			led_toggle_timeout = millis() + led_blink_ms;
		}
	}

}

byte check_door_status_hist() {
	// perform pattern matching of door status histogram
	// and return the corresponding results
	const byte allones = (1 << DOOR_STATUS_HIST_K) - 1;       // 0b1111
	const byte lowones = (1 << (DOOR_STATUS_HIST_K / 2)) - 1; // 0b0011
	const byte highones = lowones << (DOOR_STATUS_HIST_K / 2); // 0b1100

	byte _hist = door_status_hist & allones;  // get the lowest K bits
	if (_hist == 0) return DOOR_STATUS_REMAIN_CLOSED;
	if (_hist == allones) return DOOR_STATUS_REMAIN_OPEN;
	if (_hist == lowones) return DOOR_STATUS_JUST_OPENED;
	if (_hist == highones) return DOOR_STATUS_JUST_CLOSED;

	return DOOR_STATUS_MIXED;
}


void on_sta_upload_fin() {
	// finish update and check error
	if (!Update.end(true) || Update.hasError()) {
		server_send_result(HTML_UPLOAD_FAILED);
		return;
	}

	server_send_result(HTML_SUCCESS);
	restart_timeout = millis() + 2000;
}

void on_sta_upload() {
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
	delay(0);
}

// time keeping routine - the ESP module provides a millis() function 
// for general relative timekeeping, but we also want to know the
// basic UTC time... so lets maintain the UTC time in a var 
void time_keeping() {
	// if we haven't synced yet, or if we've gone too long without syncing,
	// lets synchronise time with the NTP server that we already set up.
	if (!last_utc || millis() > last_ntp + (TIME_SYNC_TIMEOUT * 1000)) {
		ulong gt = time(nullptr);

		if (gt) {
			last_utc = gt;
			last_ntp = millis();
			int drift = gt - curr_utc_time();
			oLog.info("Synchronized time using NTP. Current Unix Time=%i, Drift=%is.\r\n", gt, drift);
		}

	}
}

// periodically check the status of the door based on the sensor type
void check_status() {

	// only process if we've exceeded the read interval
	if (millis() > last_status_check + (config.getInt("read_interval") * 1000)) {
		oLog.verbose("Reading door status. ");

		last_status_check = millis();

		switch (config.getInt("sensor_type")) {

		case SENSORTYPE_ULTRASONIC_SIDE:
		case SENSORTYPE_ULTRASONIC_CEILING:
			// check the door status using an ultrasonic distance sensor
			read_value = og.read_distance();

			//not sure what this is really for...
			read_count = (read_count + 1) % 100;

			// determine based on the current reading if the door is open or closed
			door_status = (read_value > config.getInt("dth")) ? 0 : 1;

			// reverse logic for side mount
			if (config.getInt("sensor_type") == SENSORTYPE_ULTRASONIC_SIDE)
				door_status = 1 - door_status;

			break;

		case SENSORTYPE_MAGNETIC_CLOSED:
			// check the door status using a single magentic sensor in the closed position			
			read_value = door_status = digitalRead(PIN_CLOSED);

			//not sure what this is really for...
			read_count = (read_count + 1) % 255;

			break;
		}

		oLog.info("read_count=%i, read_value=%i...", read_count, read_value);

		// tack this status onto the histogram and find out what 'just happened'
		door_status_hist = (door_status_hist << 1) | door_status;
		byte event = check_door_status_hist();

		if (event == DOOR_STATUS_JUST_OPENED || event == DOOR_STATUS_JUST_CLOSED) {
			oLog.info("door status changed! door_status=%i...", door_status);
			last_status_change = curr_utc_time();

			// create a logstruct with this new status
			// info and write it to the oLog file
			LogStruct l;
			l.tstamp = last_status_change;
			l.status = door_status;
			l.value = read_value;
			og.write_log(l);

			// module : email alerts
			if ((bool)config.getString("smtp_notify_status")) {
				mail.send(config.getString("smtp_from"), config.getString("smtp_to"), "Door status changed!", String(last_status_change));
			}
		}

		oLog.info("ok!\r\n");
	}
}

/**
* Called when the physical button is pressed. When pressed we
* will reset the entire device to factory settings and reboot
**/
void onResetPress(Button& b) {
	oLog.error("Reset button depressed, resetting to factory defaults...");

	// clear the wifi configuration, delegated to the wifimanager. We must initialize a
	// new wifimanager object here as the original was already garbage collected
	oLog.error("reset wifi...");
	WiFiManager wifiManager;
	wifiManager.resetSettings();

	// clear the FS which includes the config and oLog files
	oLog.error("reset config and logs...");
	SPIFFS.format();

	oLog.error("ok!\r\n");

	// restart the ESP to establish a fresh state
	oLog.info("ESP was reset and will now be rebooted...\r\n");
	ESP.reset();
}

void setup()
{
	// initialize logging and begin output
	oLog.init(LOGLEVEL, 115200);

	oLog.info("Initializing MyGarage...\r\n");

	// initialize local clock and time servers
	oLog.verbose("Configuring NTP time servers. Server1=%s, Server2=%s...", "pool.ntp.org", "time.nist.org");
	configTime(0, 0, "pool.ntp.org", "time.nist.org", NULL);
	oLog.verbose("ok!\r\n");

	// setup the internal factory reset button
	bReset.pressHandler(onResetPress);

	// DEPRECATE: OpenGarage object
	og.begin();

	// initialize configuration object with the default config values then 
	// load the configuration from a JSON file on top for custom values
	config.setDefaultVector(defaultConfig);
	config.loadJsonFile(CONFIG_FNAME);

	// initialize a WiFiManager to establish an existing WiFi connection
	// or offer a configuration portal to the user to connect to a new
	// network, will be garbage collected after connection
	WiFiManager wifiManager;


	//set config save notify callback
	//wifiManager.setSaveConfigCallback(saveConfigCallback);

	//sets timeout until configuration portal gets turned off
	//useful to make it all retry or go to sleep (seconds)
	wifiManager.setTimeout(WIFI_PORTAL_TIMEOUT);

	// fetches previously stored ssid and password and tries to connect
	// if it cannot connect it starts an access point where the user
	// can setup the WiFi AP then goes into blocking loop waiting
	oLog.info("Auto Connecting to WiFi Network...\r\n");
	if (!wifiManager.autoConnect()) {
		oLog.info("failed to connect to AP, restarting in 30 seconds...nok!\r\n");
		delay(300000);
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(500000);
	}
	oLog.info("ok!\r\n");

	// create a server to respond to client HTTP requests on the 
	// designated port. this will be used for the JSON API as
	// well as the single page JSON application portal
	if (server) {
		oLog.verbose("Server object already existed during setup routine and has been erased.\r\n");
		delete server;
		server = NULL;
	}

	int port = config.getInt("http_port");
	oLog.info("Starting HTTP API and Application Server. Port=%i...", port);

	server = new ESP8266WebServer(port);
	server->on("/", on_get_index);
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

	oLog.info("ok!\r\n");

	// configure the SMTP mailer
	String smtp_host = config.getString("smtp_host");
	int smtp_port = config.getInt("smtp_port");
	String smtp_user = config.getString("smtp_user");
	String smtp_pass = config.getString("smtp_pass");
	mail.init(smtp_host, smtp_port, smtp_user, smtp_pass);

	// pull in last known door status so that we don't mistakenly send
	// a notification that a door event has occurred if the power was 
	// cycled to the device or if it was reset/restarted randomly
	int log_id = og.current_log_id;
	oLog.info("Reading most recent log item. LogID=%i ...", log_id);
	LogStruct current_log;
	og.read_log(current_log, log_id);
	oLog.info("got tstamp=%i, status=%i, value=%i...", current_log.tstamp, current_log.status, current_log.value);
	door_status_hist = (current_log.status == 1 ? 0b1111 : 0b0000);
	oLog.info("ok!\r\n");

	oLog.info("MyGarage has finished booting and is going into monitor mode. Read Interval=%i, Sensor Type=%i\r\n", (config.getInt("read_interval") * 1000), config.getInt("sensor_type"));

}

// The main processing loop for the application. While the
// device is powered this function will loop infinitely
// and allow us to handle processing and UI function 
void loop() {
	// allow the webserver to handle any client requests
	server->handleClient();

	// maintain the internal clock and periodically sync via NTP
	time_keeping();

	// check the status sensors and process accordingly
	check_status();

	// handle button presses, lights, and other physical UI
	process_ui();
	bReset.process();
}

//void saveConfigCallback() {
//  Serial.println("Should save config");
//}