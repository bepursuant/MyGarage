/* $Id$
||
|| @file 		       Relay.cpp
|| @author 		     Alexander Brevig              <alexanderbrevig@gmail.com>
|| @url            http://alexanderbrevig.com
||
|| @description
|| | This is a Hardware Abstraction Library for Relays
|| | It providea an easy way of handling relays
|| #
||
|| @license LICENSE_REPLACE
||
*/

#ifndef Relay_h
#define Relay_h

#include <inttypes.h>

#define RELAY_DEFAULTLOW LOW
#define RELAY_DEFAULTHIGH HIGH

#define RELAY_CLICK_MS 500

class Relay {
public:

	Relay(uint8_t relayPin);

	void click(uint8_t clickMs = RELAY_CLICK_MS);

	bool operator==(Relay &rhs);

private:
	uint8_t pin;
};

#endif