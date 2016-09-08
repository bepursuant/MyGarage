#include "SMTPClient.h"
#include "Client.h"
#include "Logging.h"

#define SMTP_PORT 25
#define ZERO_IP IPAddress(0, 0, 0, 0)
#define TIMEOUT 10000
#define CR "\r\n"

SMTPClient::SMTPClient(Client* client, char *server) : _client(client), _server(server), _serverIP(ZERO_IP), _port(SMTP_PORT) {}

SMTPClient::SMTPClient(Client* client, char *server, uint16_t port) : _client(client), _server(server), _serverIP(ZERO_IP), _port(port) {}

SMTPClient::SMTPClient(Client* client, IPAddress serverIP) : _client(client), _serverIP(serverIP), _server(""), _port(SMTP_PORT) {}

SMTPClient::SMTPClient(Client* client, IPAddress serverIP, uint16_t port) : _client(client), _serverIP(serverIP), _server(""), _port(port) {}


int SMTPClient::send(Mail *mail) {
  Log.Debug("SMTPClient::send : %s"CR, mail);

  int result = connect();
  if (!result) {
    _client->stop();
    return 0;
  }

  result = _send(mail);
  _client->stop();

  return result;
}

int SMTPClient::_send(Mail *mail) {
  if (readStatus() != 220) {
    return 0;
  }
  if (helo() != 250) {
    return 0;
  }
  if (mailFrom(mail->_from) != 250) {
    return 0;
  }
  if (rcptTo(mail) != 250) {
    return 0;
  }
  if (data() != 354) {
    return 0;
  }
  headers(mail);
  body(mail->_body);
  if (finishBody() != 250) {
    return 0;
  }
  return 1;
}

int SMTPClient::connect() {
  int result;

  if (strlen(_server) > 0)
    result = _client->connect(_server, _port);
  else
    result = _client->connect(_serverIP, _port);

  Log.Debug("SMTPClient::connect - result %i"CR, result);
  return result;
}

int SMTPClient::helo() {
  int result;

  _client->print("EHLO"CR);
  result = readStatus();

  if (result != 250) {
    // IF server doesn't understand EHLO, try HELO
    _client->print("HELO"CR);
    result = readStatus();
  }

  Log.Debug("SMTPClient::helo - result %i"CR, result);
  return result;
}

int SMTPClient::mailFrom(char *from) {
  int result;

  _client->print("MAIL FROM:");
  _client->print(from);
  _client->print(CR);

  result = readStatus();
  Log.Debug("SMTPClient::mailFrom - result %i"CR, result);
  return result;
}

int SMTPClient::rcptTo(Mail *mail) {
  int result;

  for (uint8_t i = 0; i < mail->_recipientCount; i++) {
    _client->print("RCPT TO:");
    _client->print(mail->_recipients[i]);
    _client->print(CR);

    result = readStatus();
    if (result != 250) {
      break;
    }
  }

  Log.Debug("SMTPClient::rcptTo - result %i"CR, result);
  return result;
}

int SMTPClient::data() {
  int result;

  _client->print("DATA "CR);

  result = readStatus();
  Log.Debug("SMTPClient::data - result %i"CR, result);
  return result;
}

void SMTPClient::headers(Mail *mail) {
  header("From:", mail->_from);
  if (mail->_replyTo) {
    header("Reply-To:", mail->_replyTo);
  }
  recipientHeader("To:", TO, mail);
  recipientHeader("Cc:", CC, mail);
  recipientHeader("Bcc:", BCC, mail);

  header("Subject:", mail->_subject);

  Log.Debug("SMTPClient::headers - sent"CR);
}

void SMTPClient::header(char* header, char* value) {
  _client->print(header);
  _client->print(value);
  _client->print(CR);
}

void SMTPClient::recipientHeader(char* header, recipient_t type, Mail *mail) {
  int first = 1;
  for (int i = 0; i < mail->_recipientCount; i++) {
    if (mail->_recipientTypes[i] == type) {
      if (first) {
        _client->print(header);
        first = 0;
      } else {
        _client->print(',');
      }
      _client->print(mail->_recipients[i]);
    }
  }
  if (!first) {
    _client->print(CR);
  }
}

void SMTPClient::body(char *body) {
  int cr = 0;
  int lf = 0;

  while(*body != '\0') {
    if (cr && lf && *body == '.') {
      // Add a second period to escapt the newline/period
      _client->print('.');

    }
    if (cr && *body == '\n') {
      lf = 1;
    } else if (*body == '\r') {
      cr = 1;
      lf = 0;
    } else {
      cr = lf = 0;
    }
    _client->print((char) *body++);
  }
}

int SMTPClient::finishBody() {
  int result;

  _client->print(CR);
  _client->print('.');
  _client->print(CR);

  result = readStatus();
  Log.Debug("SMTPClient::finishBody - result %i"CR, result);
  return result;
}

int SMTPClient::readStatus() {
  char line[4];
  int result;
  while(true) {
    result = readLine(line, 4);
    if (result >= 4 && (line[3] == '-')) {
      // Multiline result
      continue;
    }
    break;
  }

  if (result < 3) {
    return 0;
  }

  char st[4];
  strncpy(st, line, 3);
  st[3] = '\0';
  return atoi(st);
}

int SMTPClient::readLine(char *line, int maxLen) {
  long start = millis();
  int count = 0;
  int cr = 0;
  while (true) {
    long now = millis();
    if (now < start || now - start > TIMEOUT) {
      return 0;
    }
    int c = _client->read();
    if (c != -1) {
      if (count < maxLen) {
        line[count++] = c;
      }
      if (cr && c == '\n') {
        break;
      }
      if (c == '\r') {
        cr = 1;
        continue;
      } else {
        cr = 0;
      }
    }
  }
  if (count == maxLen - 1) {
    line[count - 1] = '\0';
  } else {
    line[count] = '\0';
  }
  return count;
}
