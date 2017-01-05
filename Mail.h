#ifndef _MAIL_h
#define _MAIL_h

#include "Log.h"
#include <ESP8266WiFi.h>
#include <base64.h>

typedef unsigned char byte;


class Mail {

public:
	String smtp_host;
	int smtp_port;
	String smtp_user;
	String smtp_pass;

	WiFiClient client;

	void init(String server, int port, String user, String password);
	byte send(String from, String to, String subject, String body);
	byte awaitResponse();
	void eFail();

private:
	bool isConfigured = false;

};

#endif