#ifndef _CONFIG_h
#define _CONFIG_h

#include "FS.h"
#include "Log.h"

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

struct Config {

	String name = DEFAULT_NAME;
	String devicekey = DEFAULT_DEVICEKEY;
	int http_port = DEFAULT_HTTP_PORT;
	bool smtp_notify_boot = DEFAULT_SMTP_NOTIFY_BOOT;
	bool smtp_notify_status = DEFAULT_SMTP_NOTIFY_BOOT;
	String smtp_host = DEFAULT_SMTP_HOST;
	int smtp_port = DEFAULT_SMTP_PORT;
	String smtp_user = DEFAULT_SMTP_USER;
	String smtp_pass = DEFAULT_SMTP_PASS;
	String smtp_from = DEFAULT_SMTP_FROM;
	String smtp_to = DEFAULT_SMTP_TO;

};

#endif