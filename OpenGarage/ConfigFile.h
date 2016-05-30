#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_

#include <ArduinoJson.h>
#include "FS.h"


class ConfigFile{
	public:
		const char* filename;

		DynamicJsonBuffer jsonBuf;
		JsonObject& json() { return jsonVar; }

		bool init(const char* filename);
		bool load();
		bool save();
		bool erase();

	private:
		JsonVariant jsonVar;
};

#endif