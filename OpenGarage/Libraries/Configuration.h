#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <ArduinoJson.h>
#include "FS.h"


// define a struct to hold configuration options
struct ConfigurationStruct {
	const char* name;
	int ival;
	String sval;

	operator int() { return this->ival; }
	operator String() { return this->sval; } 


	ConfigurationStruct(const char* name, int ival){
		this->name = name;
		this->ival = ival;
		this->sval = "";
	}

	ConfigurationStruct(const char* name, String sval){
		this->name = name;
		this->ival = 0;
		this->sval = sval;
	}
}; 


class Configuration{
	public:
		ConfigurationStruct* configStruct;
		Configuration(ConfigurationStruct*);
		ConfigurationStruct* get(const char*);
		bool set(const char*, String);
		bool set(const char*, int);
};

#endif