#include "ConfigFile.h"
#include "Logging.h"


// initialize the configfile object by reading data from
// a json file on the SPIFFS and making the json root
// object available globaly through config.json()
bool ConfigFile::init(const char* filename){

	this->filename = filename;

	return this->load();
}

bool ConfigFile::load(){
	File configFile = SPIFFS.open(this->filename, "r");

	if (!configFile) {
		Log.error("Config file %s does not exist!"CR, this->filename);
		return false;
	}

	size_t size = configFile.size();
	if (size > 1024) {
		Log.error("Config file %s is larger than %i and cannot be loaded"CR, this->filename, size);
		return false;
	}

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	String jsonStr = configFile.readString();
	
	this->jsonVar = this->jsonBuf.parseObject(jsonStr);

	if (!this->json().success()) {
		Log.error("Failed to parse config file %s"CR, this->filename);
		return false;
	}

	String retJson;
	this->json().printTo(retJson);
	Log.debug("Config file %s loaded successfully! Data %s"CR, this->filename, retJson.c_str());

	configFile.close();

	return true;
}

bool ConfigFile::save() {

	File configFile = SPIFFS.open(this->filename, "w");

	if (!configFile) {
		Log.error("Failed to open config file %s for writing"CR, this->filename);
		return false;
	}

	String retJson;
	this->json().printTo(retJson);
	Log.debug("Saving config file: %s"CR, retJson.c_str());

	this->json().printTo(configFile);
	configFile.close();

	return true;
}

bool ConfigFile::erase() {
	Log.info("Erasing config file %s!"CR, this->filename);
	return SPIFFS.remove(this->filename);
}