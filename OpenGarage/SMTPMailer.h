#ifndef SMTPMAILER_H_
#define SMTPMAILER_H_


#include "Logging.h"
#include <ESP8266WiFi.h>
#include <base64.h>

typedef unsigned char byte;


class SMTPMailer{

	public:
		const char* smtp_host;
		int smtp_port;
		const char* smtp_user;
		const char* smtp_pass;

		WiFiClient client;

		void setup(const char* server, int port, const char* user, const char* password);
		byte send(const char* from, const char* to, const char* subject, const char* body);
		byte awaitResponse();
		void eFail();

};

#endif