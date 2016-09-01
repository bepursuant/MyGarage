#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <ArduinoJson.h>
#include <vector>
#include <algorithm>
#include "FS.h"
using namespace std;

#define CONFIGTYPE_STR 0
#define CONFIGTYPE_INT 1

// define a struct to hold configuration options
struct ConfigurationStruct {
	String name;
	String sval;
	int ival;
	int type;

	ConfigurationStruct(String name, int ival){
		this->name = name;
		this->sval = String(ival);
		this->ival = ival;
	}

	ConfigurationStruct(String name, String sval){
		this->name = name;
		this->sval = sval;
		this->ival = sval.toInt();
	}

	ConfigurationStruct(String name, String sval, int ival){
		this->name = name;
		this->sval = sval;
		this->ival = ival;
	}
}; 


class Configuration{
	public:

		vector<ConfigurationStruct> vDefault;
		vector<ConfigurationStruct> vCustom;
		Configuration(const vector<ConfigurationStruct>&);
		bool setCustomVector(vector<ConfigurationStruct>&);

		String getJson();
		void setJson(String);

		ConfigurationStruct get(String);
		
		bool set(String, String);
		bool set(String, int);
		bool set(String, String, int);

};

#endif