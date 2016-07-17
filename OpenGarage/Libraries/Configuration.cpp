#include "Configuration.h"
#include "Logging.h"


// initialize the configfile object by reading data from
// a json file on the SPIFFS and making the json root
// object available globaly through Config.json()
Configuration::Configuration(ConfigurationStruct* configStruct){
	this->configStruct = configStruct;
}

// Present a way to loop through the array of configuration structs to locate one
// based on it's name and return it so that it's value can be used elsewhere.
ConfigurationStruct* Configuration::get(const char* key){
	Log.debug("Looking for %s...", key);
	int z = sizeof(this->configStruct) / sizeof(this->configStruct[0]);
	Log.debug("# of configs [%i]...", z);
	if(z > 0){
		for (int i=0;i<z;i++)
		{
			Log.debug("Config Item %s = %i/%s\r\n", this->configStruct[i].name, this->configStruct[i].ival, this->configStruct[i].sval.c_str());
			if(this->configStruct[i].name == key){
				Log.debug("Found Config Item %s = %i/%s\r\n", this->configStruct[i].name, this->configStruct[i].ival, this->configStruct[i].sval.c_str());
				return &this->configStruct[i];
			}
		}
	} else {
		Log.debug("no configs, returning null... nok!\r\n");
		return 0;
	}
}

bool Configuration::set(const char* key, String value){
	Log.verbose("Configuration Setting Key [%s].sval = %s...", key, value.c_str());
	this->get(key)->sval = value;
	Log.verbose("ok!\r\n");
	return true;
}

bool Configuration::set(const char* key, int value){
	Log.verbose("Configuration Setting Key [%s].ival = %i...", key, value);
	this->get(key)->ival = value;
	Log.verbose("ok!\r\n");
	return true;
}