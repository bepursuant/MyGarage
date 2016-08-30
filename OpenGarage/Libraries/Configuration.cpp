#include "Configuration.h"
#include "Logging.h"


// initialize the configfile object by reading data from
// a json file on the SPIFFS and making the json root
// object available globaly through config.json()
Configuration::Configuration(const vector<ConfigurationStruct>& vDefault){
	this->vDefault = vDefault;
}

bool Configuration::setCustomVector(vector<ConfigurationStruct>& vCustom){
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
	return set(key, value, 0);
}

bool Configuration::set(String key, int value){
	return set(key, String(""), value);
}

bool Configuration::set(String key, String sval, int ival){
	Log.verbose("Setting Config Value. Key=%s, sval=%s, ival=%i...", key.c_str(), sval.c_str(), ival);

	bool bSet = false;

	// first search through the default vector to make
	// sure the key we are trying to set already is
	// in our configuration. If not return false
	for (auto& vd : this->vDefault) {
		if (vd.name == key) {
			// we found it! This is a good key, lets set
			// set it's value in our custom vector
			Log.verbose("key is valid...");

			// if the value is the same as the value in the default
			// vector, we will not save it to the custom vector.
			// This prevents us from storing data we wont use
			if(vd.sval == sval.c_str() && vd.ival == ival){

				Log.verbose("and is the same as the default, ignoring...");
				bSet = true;

			} else {
			
				for (auto& vc : this->vCustom) {
					if (vc.name == key){
						Log.verbose("and already exists in custom vector...");
						// we already have a value in our custom vector,
						// all we need to do is update its values
						vc.sval = sval;
						vc.ival = ival;
						bSet = true;
						Log.verbose("and has been updated...");
						break;
					}
				}

				// the key is valid but we don't have a configstruct in our
				// custom vector yet. all we need is to create and append
				if(!bSet) {
					Log.verbose("and does not already exist...");
					ConfigurationStruct cs = ConfigurationStruct(key, sval, ival);
					this->vCustom.push_back(cs);
					bSet = true;
					Log.verbose("but has been created...");
				}
			}
			break;
		}
	}

	if(bSet){
		Log.verbose("ok!\r\n");
		return true;
	} else {
		Log.verbose("nok!\r\n");
		return false;
	}
}

// Transform the custom vector into a JSON string that
// can be saved to file or sent as an http response
String Configuration::getJson(){

	Log.verbose("Serializing custom configuration into a JSON string...");

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonArray& config = root.createNestedArray("config");

	for (auto& vc : this->vCustom) {
		JsonObject& cfg = config.createNestedObject();
		cfg["name"] = vc.name.c_str();
		cfg["sval"] = vc.sval.c_str();
		cfg["ival"] = vc.ival;
	}

	String retJson;
	root.printTo(retJson);

	Log.verbose("JSON=%s...ok!\r\n", retJson.c_str());

	return retJson;
}

void Configuration::setJson(String json){
	Log.verbose("Loading configuration from JSON. JSON=%s...", json.c_str());

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json);

	// Test if parsing succeeds.
	if (!root.success()) {
		Log.verbose("parse failed...nok!\r\n");
		return;
	} else {
		Log.verbose("parsed successfully...ok!\r\n");
	}

	for (auto& js : root){
		Log.verbose("Loading config item. Name=%s, sval=%s, ival=%i...", js.name.c_str(), js.sval.c_str(), js.ival);

		ConfigurationStruct cs = ConfigurationStruct(js.name, js.sval, js.ival);
		this->vCustom.push_back(cs);

		Log.verbose("ok!\r\n");
	}
}