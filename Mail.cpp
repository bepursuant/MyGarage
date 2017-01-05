// Mail provides easy methods to send emails from our sketch by
// connecting to an SMTP server and sending the email directly.
// Original Email client sketch for IDE v1.0.5 and w5100/w5200
// Posted 7 May 2015 by SurferTim

#include "Mail.h"

void Mail::init(String server, int port, String user, String password) {
	oLog.verbose("Initializing Mail object. Server=%s, Port=%i, User=%s, Password=%s...", server.c_str(), port, user.c_str(), password.c_str());
	if (!server || !port || !user || !password) {
		oLog.error("Unable to initialize Mail with given parameters...nok!\r\n");
		return;
	}

	this->smtp_host = server;
	this->smtp_port = port;
	this->smtp_user = user;
	this->smtp_pass = password;
	this->isConfigured = true;

	oLog.verbose("ok!\r\n");
	return;
}

byte Mail::send(String from, String to, String subject, String body)
{
	oLog.verbose("Sending email. From=%s, To=%s, Subject=%s, Body=%s...", from.c_str(), to.c_str(), subject.c_str(), body.c_str());
	if (!isConfigured) {
		oLog.error("Mail object not configured, make sure to call init()...nok!\r\n");
		return false;
	}

	if (!from || !to || !subject || !body) {
		oLog.error("Send parameters are all required, make sure to send values...nok!\r\n");
		return false;
	}

	// connect wificlient to the SMTP server
	if (!client.connect(this->smtp_host.c_str(), this->smtp_port)) {
		oLog.error("Failed to connect to SMTP Host. Server=%s, Port=%i...nok!\r\n", this->smtp_host.c_str(), this->smtp_port);
		return 0;
	}

	oLog.verbose("connected to SMTP host. Server=%s, Port=%i...", this->smtp_host.c_str(), this->smtp_port);
	if (!awaitResponse()) return 0; // wait up to 10 seconds for a response, or bail

	oLog.verbose("EHLO %s...", this->smtp_host.c_str());
	client.printf("EHLO %s\r\n", this->smtp_host.c_str());	//change to your public ip
	if (!awaitResponse()) return 0;

	oLog.verbose("AUTH LOGIN...");
	client.println("AUTH LOGIN");
	if (!awaitResponse()) return 0;

	oLog.verbose("USER %s...", this->smtp_user.c_str());
	client.println(base64::encode(this->smtp_user));
	if (!awaitResponse()) return 0;

	oLog.verbose("PASS %s...", this->smtp_pass.c_str());
	client.println(base64::encode(this->smtp_pass));
	if (!awaitResponse()) return 0;

	oLog.verbose("MAIL From: <%s>...", from.c_str());
	client.printf("MAIL From: <%s>\r\n", from.c_str());	// change to your email address (sender)
	if (!awaitResponse()) return 0;

	oLog.verbose("RCPT To: <%s>..", to.c_str());
	client.printf("RCPT To: <%s>\r\n", to.c_str());	// change to recipient address
	if (!awaitResponse()) return 0;

	oLog.verbose("DATA...");
	client.write("DATA\r\n");
	if (!awaitResponse()) return 0;

	client.printf("To: %s\r\n", to.c_str());	// change to recipient address
	client.printf("From: %s\r\n", from.c_str());
	client.printf("Subject: %s\r\n", subject.c_str());
	client.printf("%s\r\n", body.c_str());
	client.write(".\r\n");
	if (!awaitResponse()) return 0;

	oLog.verbose("QUIT...");
	client.write("QUIT\r\n");
	if (!awaitResponse()) return 0;

	client.stop();
	oLog.verbose("message sent...ok!\r\n");

	return 1;
}

byte Mail::awaitResponse()
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;

	while (!client.available()) {
		delay(1);
		loopCount++;

		delay(0);

		// if nothing received for 10 seconds, timeout
		if (loopCount > 10000) {
			client.stop();
			oLog.verbose("\r\nTimeout\r\n");
			return 0;
		}
	}

	respCode = client.peek();

	while (client.available())
	{
		thisByte = client.read();
		Serial.write(thisByte);
	}

	if (respCode >= '4')
	{
		eFail();
		return 0;
	}

	return 1;
}

void Mail::eFail()
{
	byte thisByte = 0;
	int loopCount = 0;

	client.write("QUIT\r\n");

	while (!client.available()) {
		delay(1);
		loopCount++;

		// if nothing received for 10 seconds, timeout
		if (loopCount > 10000) {
			client.stop();
			oLog.verbose("\r\nTimeout\r\n");
			return;
		}
	}

	while (client.available())
	{
		thisByte = client.read();
		Serial.write(thisByte);
	}

	client.stop();
	oLog.verbose("Disconnected\r\n");
}