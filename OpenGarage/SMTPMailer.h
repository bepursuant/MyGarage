#ifndef SMTPMAILER_H_
#define SMTPMAILER_H_


#include "Logging.h"
#include <ESP8266WiFi.h>
#include <base64.h>

typedef unsigned char byte;


class SMTPMailer{

	public:
		char* smtp_host;
		int smtp_port;
		char* smtp_user;
		char* smtp_pass;

		WiFiClient client;

		void setup(char* server, int port, char* user, char* password);
		byte send(char* from, char* to, char* subject, char* body);
		byte awaitResponse();
		void eFail();

};

#endif