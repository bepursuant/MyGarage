/* $Id$
||
|| @file 		       Led.cpp
|| @author 		     Alexander Brevig              <alexanderbrevig@gmail.com>
|| @url            http://alexanderbrevig.com
||
|| @description
|| | This is a Hardware Abstraction Library for Leds
|| | It providea an easy way of handling leds
|| #
||
|| @license LICENSE_REPLACE
||
*/

#ifndef Led_h
#define Led_h

#include <FastLED.h>

struct LedState : CRGB {
};

class Led {

public:
	void set(LedState ledState);

};

#endif