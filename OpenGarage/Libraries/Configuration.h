#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <ArduinoJson.h>
#include <vector>
#include "FS.h"
using namespace std;

// define a struct to hold configuration options
struct ConfigurationStruct {
	String name;
	String sval;
	int ival;

	ConfigurationStruct(String name, int ival){
		this->name = name;
		this->ival = ival;
	}

	ConfigurationStruct(String name, String sval){
		this->name = name;
		this->sval = sval;
	}
}; 


class Configuration{
	public:

		vector<ConfigurationStruct> vDefault;
		vector<ConfigurationStruct> vCustom;
		Configuration(const vector<ConfigurationStruct>&);
		bool setCustomVector(const vector<ConfigurationStruct>&);

		ConfigurationStruct get(String);
		
		bool set(String, String);
		bool set(String, int);
};

#endif