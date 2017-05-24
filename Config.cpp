// KeyValueStore provides a way of storing key value combinations
// and persisting that value to a json file. In this way, it acts
// as a good configuration keeper object

#include "Config.h"
#include "Log.h"
#include <vector>

bool Config::setDefaultVector(const vector<ConfigStruct>& vDefault) {
	this->vDefault = vDefault;
	return true;
}

bool Config::setCustomVector(vector<ConfigStruct>& vCustom) {
	this->vCustom = vCustom;
	return true;
}

// These functions provide outward facing functionality to the 
// implementing code. They simply call the get function and
// return the sval or ival depending on which was called
String Config::getString(String key) {
	return this->get(key).sval;
}

int Config::getInt(String key) {
	return this->get(key).ival;
}

const char* Config::getChar(String key) {
	return this->get(key).sval.c_str();
}

// Present a way to loop through the array of keyvalue structs to locate one
// based on its key and return it so that its value can be used elsewhere
ConfigStruct Config::get(String key) {
	//oLog.verbose("Looking for key=%s...", key.c_str());

	// First search through the custom vector to see if we have an 
	// overridden value, possibly loaded from a file or other ui
	for (const auto& cs : this->vCustom) {
		if (cs.key == key) {
			//oLog.verbose("found it in custom vector...ival=%i, sval=%s...ok!\r\n", cs.ival, cs.sval.c_str());
			return cs;
		}
	}

	// Next look through default to see if we have a value set
	for (const auto& cs : this->vDefault) {
		if (cs.key == key) {
			//oLog.verbose("found it in default vector...ival=%i, sval=%s...ok!\r\n", cs.ival, cs.sval.c_str());
			return cs;
		}
	}

	//oLog.verbose("not found, returning null default...nok!\r\n");
	return ConfigStruct({ key,0 });
}

bool Config::set(String key, String value) {
	return set(key, value, value.toInt());
}

bool Config::set(String key, int value) {
	return set(key, String(value), value);
}

bool Config::set(String key, String sval, int ival) {
	oLog.verbose("Setting KeyValue. Key=%s, sval=%s, ival=%i...", key.c_str(), sval.c_str(), ival);

	bool bSet = false;

	// first search through the custom vector to see
	// if we already have a value stored for key,
	// if not, we will save it as a new value
	for (auto& vc : this->vCustom) {

		if (vc.key == key) {
			// we already have a value stored for this key
			oLog.verbose("keyvalue exists in the custom vector...");


			// we only want to save a value if its not the same
			// as the value in the default vector. why store
			// a value that we already know?
			for (auto& vd : this->vDefault) {
				if (vd.key == key && vd.sval == sval && vd.ival == ival) {
					oLog.verbose("but is now the same as the default value...");
					// erase-remove the value if it is in the custom
					// vector as it is being set to the default
					this->vCustom.erase(
						remove_if(
							this->vCustom.begin(),
							this->vCustom.end(),
							[&key](ConfigStruct cs) {
						return cs.key == key;
					}),
						this->vCustom.end()
						);

					bSet = true;
					break;
				}
			}

			// the key already exists in the custom vector and the new value
			// does not match the default value, so we just do an update
			if (!bSet) {
				vc.sval = sval;
				vc.ival = ival;

				oLog.verbose("and has been updated...");
				bSet = true;
				break; // found it, no need to search further
			}

			break; // found it, no need to search further
		}

	}

	// the key is valid but we don't have a configstruct in our
	// custom vector yet. all we need is to create and append
	if (!bSet) {
		oLog.verbose("did not exist in the custom vector...");
		ConfigStruct cs = ConfigStruct(key, sval, ival);
		this->vCustom.push_back(cs);
		bSet = true;
		oLog.verbose("but has been added...");
	}

	if (bSet) {
		oLog.verbose("ok!\r\n");
		return true;
	}
	else {
		oLog.verbose("nok!\r\n");
		return false;
	}
}

// Transform the custom vector into a JSON string that
// can be saved to file or sent as an http response
String Config::getJson() {
	oLog.verbose("Serializing custom configuration into a JSON string...");

	DynamicJsonBuffer jsonBuffer;
	JsonArray& root = jsonBuffer.createArray();

	for (auto& vc : this->vCustom) {
		JsonObject& kvp = root.createNestedObject();
		kvp["key"] = vc.key.c_str();
		kvp["sval"] = vc.sval.c_str();
		kvp["ival"] = vc.ival;
	}

	String retJson;
	root.printTo(retJson);

	oLog.verbose("JSON=%s...ok!\r\n", retJson.c_str());
	return retJson;
}

bool Config::setJson(String json) {
	oLog.verbose("Loading KeyValues from Json. JSON=%s...", json.c_str());

	DynamicJsonBuffer jsonBuffer;
	JsonArray& root = jsonBuffer.parseArray(json);

	// Test if parsing succeeds.
	if (!root.success()) {
		oLog.error("Failed to parse configuration file...nok!\r\n");
		return false;
	}

	oLog.verbose("file parsed successfully...");

	this->vCustom.clear(); // clear the vector so we can load it from scratch
	for (auto& kvp : root) {

		String key = kvp["key"].asString();
		String sval = kvp["sval"].asString();
		int ival = kvp["ival"].as<int>();

		oLog.verbose("Loading keyvalue. Key=%s, sval=%s, ival=%i...", key.c_str(), sval.c_str(), ival);

		ConfigStruct cs = ConfigStruct(key, sval, ival);
		this->vCustom.push_back(cs);

		oLog.verbose("ok!\r\n");

	}

	oLog.verbose("ok!\r\n");
	return true;
}


bool Config::loadJsonFile(String filename) {
	oLog.verbose("Loading KeyValues from Json File. Filename=%s...", filename.c_str());

	File jsonFile = SPIFFS.open(filename, "r");

	if (!jsonFile) {
		oLog.error("Failed to open KeyValue file...nok!\r\n");
		return false;
	}

	size_t size = jsonFile.size();
	if (size > 1024) {
		oLog.error("KeyValue file size is too large. Size limit=%i...nok!\r\n", 1024);
		return false;
	}

	// Allocate a buffer to store contents of the file.
	//std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use jsonFile.readString instead.
	this->setJson(String(jsonFile.readString()));

	oLog.verbose("ok!\r\n");
	return true;
}

bool Config::saveJsonFile(String filename) {
	oLog.verbose("Saving KeyValues to Json File. Filename=%s...", filename.c_str());

	File jsonFile = SPIFFS.open(filename, "w");

	if (!jsonFile) {
		oLog.error("Failed to open KeyValue file for writing...nok!\r\n");
		return false;
	}

	jsonFile.print(this->getJson());
	jsonFile.close();
	oLog.verbose("ok!\r\n");
	return true;
}