#include "Configuration.h"
#include "Logging.h"


// initialize the configfile object by reading data from
// a json file on the SPIFFS and making the json root
// object available globaly through Config.json()
Configuration::Configuration(const vector<ConfigurationStruct>& configStruct){
	this->configStruct = configStruct;
}

// Present a way to loop through the array of configuration structs to locate one
// based on it's name and return it so that it's value can be used elsewhere.
ConfigurationStruct Configuration::get(String key){
	//Log.verbose("Looking for %s...", key.c_str());
	for (const auto& cs : configStruct) {
		if (cs.name == key) {
			//Log.verbose("found it...ival=%i, sval=%s...ok!\r\n", cs.ival, cs.sval.c_str());
			return cs;
		}
	}
	
	//Log.verbose("not found, returning null default...nok!\r\n");
	return ConfigurationStruct({key,0});
}

bool Configuration::set(String key, String value){
	Log.verbose("Configuration Setting Key [%s].sval = %s...", key.c_str(), value.c_str());
	for (const auto& cs : configStruct) {
		if (cs.name == key) {
			//cs.sval = value;
			break;
		}
	}
	Log.verbose("ok!\r\n");
	return true;
}

bool Configuration::set(String key, int value){
	Log.verbose("Configuration Setting Key [%s].ival = %i...", key.c_str(), value);

	for (const auto& cs : configStruct) {
		if (cs.name == key) {
			//cs.ival = value;
			break;
		}
	}
	Log.verbose("ok!\r\n");
	return true;
}