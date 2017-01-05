#ifndef _CONFIG_h
#define _CONFIG_h

#include <ArduinoJson.h>
#include <vector>
#include <algorithm>
#include "FS.h"
using namespace std;

// define a struct to hold configuration options
struct ConfigStruct {
	String key;
	String sval;
	int ival;
	int type;

	ConfigStruct(String key, int ival) {
		this->key = key;
		this->sval = String(ival);
		this->ival = ival;
	}

	ConfigStruct(String key, String sval) {
		this->key = key;
		this->sval = sval;
		this->ival = sval.toInt();
	}

	ConfigStruct(String key, String sval, int ival) {
		this->key = key;
		this->sval = sval;
		this->ival = ival;
	}
};


class Config {
public:

	bool setDefaultVector(const vector<ConfigStruct>&);
	bool setCustomVector(vector<ConfigStruct>&);

	int getInt(String);
	String getString(String);

	bool set(String, String);
	bool set(String, int);

	String getJson();
	bool loadJsonFile(String);
	bool saveJsonFile(String);

private:
	vector<ConfigStruct> vDefault;
	vector<ConfigStruct> vCustom;

	ConfigStruct get(String);

	bool setJson(String);
	bool set(String, String, int);

};

#endif