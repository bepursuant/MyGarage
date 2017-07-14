#include <Arduino.h>
#include "Relay.h"


Relay::Relay(uint8_t relayPin) {
	pin = relayPin;
	digitalWrite(pin, LOW);
	pinMode(pin, OUTPUT);
}

void Relay::click(uint8_t clickMs) {
	digitalWrite(pin, HIGH);
	delay(clickMs);
	digitalWrite(pin, LOW);
}