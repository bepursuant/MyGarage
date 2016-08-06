#include "Configuration.h"
#include "Logging.h"


// initialize the configfile object by reading data from
// a json file on the SPIFFS and making the json root
// object available globaly through config.json()
Configuration::Configuration(const vector<ConfigurationStruct>& vDefault){
	this->vDefault = vDefault;
}

bool Configuration::setCustomVector(const vector<ConfigurationStruct>& vCustom){
	this->vCustom = vCustom;
	return true;
}

// Present a way to loop through the array of configuration structs to locate one
// based on it's name and return it so that it's value can be used elsewhere.
ConfigurationStruct Configuration::get(String key){
	//Log.verbose("Looking for key=%s...", key.c_str());

	// First search through the custom vector to see if we have an 
	// overridden value, possibly loaded from a file or ui
	for (const auto& cs : this->vCustom) {
		if (cs.name == key) {
			//Log.verbose("found it in custom vector...ival=%i, sval=%s...ok!\r\n", cs.ival, cs.sval.c_str());
			return cs;
		}
	}

	// Next look through default to see if we have a value set
	for (const auto& cs : this->vDefault) {
		if (cs.name == key) {
			//Log.verbose("found it in default vector...ival=%i, sval=%s...ok!\r\n", cs.ival, cs.sval.c_str());
			return cs;
		}
	}
	
	//Log.verbose("not found, returning null default...nok!\r\n");
	return ConfigurationStruct({key,0});
}

bool Configuration::set(String key, String value){
	Log.verbose("Configuration Setting Key [%s].sval = %s...", key.c_str(), value.c_str());
	for (const auto& cs : this->vCustom) {
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

	for (const auto& cs : this->vCustom) {
		if (cs.name == key) {
			//cs.ival = value;
			break;
		}
	}
	Log.verbose("ok!\r\n");
	return true;
}