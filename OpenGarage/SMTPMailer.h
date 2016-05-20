#include <SPI.h>
#include <Ethernet.h>


class SMTPMailer{

	const char* server = "smtp.google.com";
	int port = 2525;


	public:
		void setup(char* server, int port);
		byte sendEmail(char to, char from, char subject, char body);

	private:
		EthernetClient client;
		byte eRcv();
		void eFail();

};