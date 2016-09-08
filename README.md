
This folder contains the MyGarage firmware code for Arduino with ESP8266.

Requirements
===========

* Arduino IDE (newest version)
* ESP8266 library for Arduino (installation instructions at https://github.com/esp8266/Arduino)

Compilation
===========

* Connect your device
* Open MyGarage.ino in the Arduino IDE
* Select the correct board under Tools >> Board >> Generic ESP8266 Module (if this is not available, check if you've installed the ESP8266 library for Arduino)
* Select Tools >> Flash Size >> 4M (1M SPIFFS).
* Click the (=>) button to build and upload to your device

Initial Connection
===========

On first boot-up, the firmware goes into AP mode, creating a WiFi network named ESP{chip_id}. This is an open AP with no password. Using your phone or computer to connect to this AP, then open a browser and type in

192.168.4.1

to access the AP homepage. Select (or manually type in) the desired SSID, password and submit. The device will connect to your router and get an IP address.

Login to the admin panel using the IP address assigned by your router - the default devicekey is "opendoor".