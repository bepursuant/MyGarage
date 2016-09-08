/*
	 Email client sketch for IDE v1.0.5 and w5100/w5200
	 Posted 7 May 2015 by SurferTim
*/

#include "SMTPMailer.h"

void SMTPMailer::init(String server, int port, String user, String password){
	Log.verbose("Initializing SMTPMailer object. Server=%s, Port=%i, User=%s, Password=%s...", server.c_str(), port, user.c_str(), password.c_str());
	if(!server || !port || !user || !password){
		Log.error("Unable to initialize SMTPMailer with given parameters...nok!\r\n");
		return;
	}

	this->smtp_host = server;
	this->smtp_port = port;
	this->smtp_user = user;
	this->smtp_pass = password;

	Log.verbose("ok!\r\n");
	return;
}

byte SMTPMailer::send(String from, String to, String subject, String body)
{
	Log.verbose("Sending email. From=%s, To=%s, Subject=%s, Body=%s...", from.c_str(), to.c_str(), subject.c_str(), body.c_str());
	
	if(!from || !to || !subject || !body){
		Log.error("Unable to send email with given parameters...nok!\r\n");
		return false;
	}

	byte thisByte = 0;
	byte respCode;

	if(!client.connect(this->smtp_host.c_str(), this->smtp_port)) {
		Log.error("Failed to connect to SMTP Host. Server=%s, Port=%i...nok!\r\n", this->smtp_host.c_str(), this->smtp_port);
		return 0;
	}

	Log.verbose("connected to SMTP host. Server=%s, Port=%i...", this->smtp_host.c_str(), this->smtp_port);
	if(!awaitResponse()) return 0;

	Log.verbose("EHLO %s...", this->smtp_host.c_str());
	client.printf("EHLO %s\r\n", this->smtp_host.c_str());	//change to your public ip
	if(!awaitResponse()) return 0;

	Log.verbose("AUTH LOGIN...");
	client.println("AUTH LOGIN");
	if(!awaitResponse()) return 0;

	Log.verbose("USER %s...",this->smtp_user.c_str());
	client.println(base64::encode(this->smtp_user));
	if(!awaitResponse()) return 0;

	Log.verbose("PASS %s...", this->smtp_pass.c_str());
	client.println(base64::encode(this->smtp_pass));
	if(!awaitResponse()) return 0;

	Log.verbose("MAIL From: <%s>...",from.c_str());
	client.printf("MAIL From: <%s>\r\n", from.c_str());	// change to your email address (sender)
	if(!awaitResponse()) return 0;

	Log.verbose("RCPT To: <%s>..", to.c_str());
	client.printf("RCPT To: <%s>\r\n", to.c_str());	// change to recipient address
	if(!awaitResponse()) return 0;

	Log.verbose("DATA...");
	client.write("DATA\r\n");
	if(!awaitResponse()) return 0;

	client.printf("To: %s\r\n", to.c_str());	// change to recipient address
	client.printf("From: %s\r\n", from.c_str());
	client.printf("Subject: %s\r\n", subject.c_str());
	client.printf("%s\r\n", body.c_str());
	client.write(".\r\n");
	if(!awaitResponse()) return 0;

	Log.verbose("QUIT...");
	client.write("QUIT\r\n");
	if(!awaitResponse()) return 0;

	client.stop();
	Log.verbose("message sent...ok!\r\n");

	return 1;
}

byte SMTPMailer::awaitResponse()
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;

	while(!client.available()) {
		delay(1);
		loopCount++;

		delay(0);

		// if nothing received for 10 seconds, timeout
		if(loopCount > 10000) {
			client.stop();
			Log.verbose("\r\nTimeout\r\n");
			return 0;
		}
	}

	respCode = client.peek();

	while(client.available())
	{  
		thisByte = client.read();    
		Serial.write(thisByte);
	}

	if(respCode >= '4')
	{
		eFail();
		return 0;  
	}

	return 1;
}

void SMTPMailer::eFail()
{
	byte thisByte = 0;
	int loopCount = 0;

	client.write("QUIT\r\n");

	while(!client.available()) {
		delay(1);
		loopCount++;

		// if nothing received for 10 seconds, timeout
		if(loopCount > 10000) {
			client.stop();
			Log.verbose("\r\nTimeout\r\n");
			return;
		}
	}

	while(client.available())
	{  
		thisByte = client.read();    
		Serial.write(thisByte);
	}

	client.stop();
	Log.verbose("Disconnected\r\n");
}