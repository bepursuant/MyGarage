#ifndef _CONFIG_h
#define _CONFIG_h

#include "FS.h"
#include "Log.h"
#include "SerializeEeprom.h"

#define DEFAULT_NAME		"MyGarage"
#define DEFAULT_DEVICEKEY	"opendoor"
#define DEFAULT_HTTP_PORT			80
#define DEFAULT_SMTP_NOTIFY_BOOT	0
#define DEFAULT_SMTP_NOTIFY_STATUS	0
#define DEFAULT_SMTP_HOST	""
#define DEFAULT_SMTP_PORT	587
#define DEFAULT_SMTP_USER	""
#define DEFAULT_SMTP_PASS	""
#define DEFAULT_SMTP_FROM	""
#define DEFAULT_SMTP_TO		""
#define DEFAULT_AP_SSID		""
#define DEFAULT_AP_PASS		""

class Config {

public:
	char name[32] = DEFAULT_NAME;
	char devicekey[255] = DEFAULT_DEVICEKEY;
	int http_port = DEFAULT_HTTP_PORT;
	bool smtp_notify_boot = DEFAULT_SMTP_NOTIFY_BOOT;
	bool smtp_notify_status = DEFAULT_SMTP_NOTIFY_BOOT;
	char smtp_host[255] = DEFAULT_SMTP_HOST;
	int smtp_port = DEFAULT_SMTP_PORT;
	char smtp_user[255] = DEFAULT_SMTP_USER;
	char smtp_pass[255] = DEFAULT_SMTP_PASS;
	char smtp_from[255] = DEFAULT_SMTP_FROM;
	char smtp_to[255] = DEFAULT_SMTP_TO;
	char ap_ssid[32] = DEFAULT_AP_SSID;
	char ap_pass[255] = DEFAULT_AP_PASS;

};

// define the SAVE implementation for the above structure
bool SaveConfig(Config &cfg, unsigned char logLevel = EEPROM_log_RW)
{
	return eepromIf<Config>::Save(&cfg, logLevel);
}

// define the LOAD implementation for the above structure
bool LoadConfig(Config &cfg, unsigned char logLevel = EEPROM_log_RW)
{
	return eepromIf<Config>::Load(&cfg, logLevel);
}

template<> int eepromIf<Config>::Signature = 0x43534553;
template<> int eepromIf<Config>::baseOffset = 0;




#endif