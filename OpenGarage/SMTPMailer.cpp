/*
	 Email client sketch for IDE v1.0.5 and w5100/w5200
	 Posted 7 May 2015 by SurferTim
*/

#include "SMTPMailer.h"

void SMTPMailer::setup(char* server, int port, char* user, char* password){
	this->smtp_host = server;
	this->smtp_port = port;
	this->smtp_user = user;
	this->smtp_pass = password;
}

byte SMTPMailer::send(char* from, char* to, char* subject, char* body)
{
	Log.Info("Sending email From [%s] To [%s] Subject[%s]", from, to, subject);

	byte thisByte = 0;
	byte respCode;

	if(client.connect(this->smtp_host, this->smtp_port)) {
		Log.Verbose("Connected to Host:%s Port:%i"CR, this->smtp_host, this->smtp_port);
	} else {
		Log.Error("Could not connect to Host [%s] Port:[%i]"CR, this->smtp_host, this->smtp_port);
		return 0;
	}

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending EHLO"CR);
	client.printf("EHLO %s\r\n", this->smtp_host);	//change to your public ip

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending AUTH LOGIN"CR);
  client.println("AUTH LOGIN");
  if(!awaitResponse()) return 0;
 
  Log.Verbose("Sending User"CR);
  client.println(base64::encode(this->smtp_user));
 
  if(!awaitResponse()) return 0;
 
  Log.Verbose("Sending Password"CR);
  client.println(base64::encode(this->smtp_pass));
 
  if(!awaitResponse()) return 0;

	Log.Verbose("Sending From"CR);
	client.printf("MAIL From: <%s>\r\n", from);	// change to your email address (sender)

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending To"CR);
	client.printf("RCPT To: <%s>\r\n", to);	// change to recipient address

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending DATA"CR);
	client.write("DATA\r\n");

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending Message"CR);

	client.printf("To: %s\r\n", to);	// change to recipient address

	client.printf("From: %s\r\n", from);

	client.printf("Subject: %s\r\n", subject);

	client.println("Test Body");

	client.write(".\r\n");

	Log.Debug("Message Sent"CR);

	if(!awaitResponse()) return 0;

	Log.Verbose("Sending QUIT"CR);
	client.write("QUIT\r\n");

	if(!awaitResponse()) return 0;

	client.stop();
	Log.Verbose("Disconnected"CR);

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
			Log.Verbose("\r\nTimeout"CR);
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
			Log.Verbose("\r\nTimeout"CR);
			return;
		}
	}

	while(client.available())
	{  
		thisByte = client.read();    
		Serial.write(thisByte);
	}

	client.stop();
	Log.Verbose("Disconnected"CR);
}