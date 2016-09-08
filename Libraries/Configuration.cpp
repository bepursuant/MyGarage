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
	// overridden value, possibly loaded from a file or other ui
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
			Log.verbose("exists in the default vector...");


			// we only want to save a value if its not the same
			// as the value in the default vector. why store
			// a value that we already know?
			if(vd.sval == sval && vd.ival == ival){
				Log.verbose("is the same as the default...");
				bSet = true;

				// erase-remove the value if it is in the custom
				// vector as it is being set to the default
				//bool nameMatch(const ConfigurationStruct& cs){
				//   return cs.name == key;
				//}
				this->vCustom.erase(
					remove_if(
						this->vCustom.begin(),
						this->vCustom.end(),
						[&key](ConfigurationStruct cs){
							return cs.name == key;
						}),
					this->vCustom.end()
				);
			}


			// now iterate the custom vector to see if the 
			// key is already stored here. If it is, then
			// update the value, otherwise create it
			if(!bSet){
				for (auto& vc : this->vCustom) {

					if (vc.name == key){

						// we already have a value in our custom vector,
						// all we need to do is update its values
						Log.verbose("already exists in custom vector...");

						// if the value is the same as the value in the default
						// vector, we will not save it to the custom vector.
						// This prevents us from storing data we wont use
						vc.sval = sval;
						vc.ival = ival;
						bSet = true;
						Log.verbose("and has been updated...");

						break; // found it, no need to search further
					}
				}
			}

			// the key is valid but we don't have a configstruct in our
			// custom vector yet. all we need is to create and append
			if(!bSet){
				Log.verbose("is not overridden...");
				ConfigurationStruct cs = ConfigurationStruct(key, sval, ival);
				this->vCustom.push_back(cs);
				bSet = true;
				Log.verbose("but has been saved...");
			}

			break; // found it, no need to search further
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

bool Configuration::setJson(String json){
	Log.verbose("Loading configuration from JSON. JSON=%s...", json.c_str());

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(json);

	// Test if parsing succeeds.
	if (!root.success()) {
		Log.error("Failed to parse configuration file...nok!\r\n");
		return false;
	}

	Log.verbose("file parsed successfully...");

	JsonArray& config = root["config"];
	this->vCustom.clear(); // clear the vector so we can load it from scratch
	for (auto& js : config){
		String name = js["name"].asString();
		String sval = js["sval"].asString();
		int ival = js["ival"].as<int>();

		Log.verbose("Loading config item. Name=%s, sval=%s, ival=%i...", name.c_str(), sval.c_str(), ival);

		ConfigurationStruct cs = ConfigurationStruct(name, sval, ival);
		this->vCustom.push_back(cs);

		Log.verbose("ok!\r\n");
	}

	Log.verbose("ok!\r\n");
	return true;
}


bool Configuration::loadJsonFile(String jsonFilePath) {
	Log.verbose("Loading configuration from json file. Filepath=%s...", jsonFilePath.c_str());
	
	File jsonFile = SPIFFS.open(jsonFilePath, "r");
	if (!jsonFile) {
		Log.error("Failed to open configuration file...nok!\r\n");
		return false;
	}

	size_t size = jsonFile.size();
	if (size > 1024) {
		Log.error("Config file size is too large. Size limit=%i...nok!\r\n", 1024);
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use jsonFile.readString instead.
	this->setJson(String(jsonFile.readString()));
	Log.verbose("ok!\r\n");
	return true;
}

bool Configuration::saveJsonFile(String jsonFilePath) {
	Log.verbose("Saving configuration to file. Filepath=%s...", jsonFilePath.c_str());

	File jsonFile = SPIFFS.open(jsonFilePath, "w");

	if (!jsonFile) {
		Log.error("Failed to open config file for writing...nok!\r\n");
		return false;
	}

	jsonFile.print(this->getJson());
	jsonFile.close();
	Log.verbose("ok!\r\n");
	return true;
}