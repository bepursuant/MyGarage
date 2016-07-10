#ifndef _LOGGING_H
#define _LOGGING_H

#include <inttypes.h>
#include <stdarg.h>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//#include "pins_arduino.h"

#define LOGGING_NOOUTPUT 0 
#define LOGGING_ERROR 1
#define LOGGING_INFO 2
#define LOGGING_DEBUG 3
#define LOGGING_VERBOSE 4

#define CR "\r\n"
#define LOG_VERSION 1

/*!
* Logging is a helper class to output informations over
* RS232. If you know log4j or log4net, this logging class
* is more or less similar ;-) <br>
* Different loglevels can be used to extend or reduce output
* All methods are able to handle any number of output parameters.
* All methods print out a formated string (like printf).<br>
* To reduce output and program size, reduce loglevel.
* <br>
* Output format string can contain below wildcards. Every wildcard
* must be start with percent sign (\%)
*
* <b>Depending on loglevel, source code is excluded from compile !</b><br>
* <br>
* <b>Wildcards</b><br>
* <ul>
* <li><b>\%s</b>	replace with an string (const char*)</li>
* <li><b>\%c</b>	replace with an character</li>
* <li><b>\%d</b>	replace with an integer value</li>
* <li><b>\%l</b>	replace with an long value</li>
* <li><b>\%x</b>	replace and convert integer value into hex</li>
* <li><b>\%X</b>	like %x but combine with <b>0x</b>123AB</li>
* <li><b>\%b</b>	replace and convert integer value into binary</li>
* <li><b>\%B</b>	like %x but combine with <b>0b</b>10100011</li>
* <li><b>\%t</b>	replace and convert boolean value into <b>"t"</b> or <b>"f"</b></li>
* <li><b>\%T</b>	like %t but convert into <b>"true"</b> or <b>"false"</b></li>
* </ul><br>
* <b>Loglevels</b><br>
* <table border="0">
* <tr><td>0</td><td>LOGLEVEL_NOOUTPUT</td><td>no output </td></tr>
* <tr><td>1</td><td>LOGLEVEL_ERRORS</td><td>only errors </td></tr>
* <tr><td>2</td><td>LOGLEVEL_INFOS</td><td>errors and info </td></tr>
* <tr><td>3</td><td>LOGLEVEL_DEBUG</td><td>errors, info and debug </td></tr>
* <tr><td>4</td><td>LOGLEVEL_VERBOSE</td><td>all </td></tr>
* </table>
* <br>
* <h1>History</h1><br>
* <table border="0">
* <tr><td>01 FEB 2012</td><td>initial release</td></tr>
* <tr><td>06 MAR 2012</td><td>implement a preinstanciate object (like in Wire, ...)</td></tr>
* <tr><td></td><td>methode init get now loglevel and baud parameter</td></tr>
*/
class Logging {
private:
    int _level;
    long _baud;
    
public:
    /*! 
	 * default Constructor
	 */
    Logging(){} ;
	
    /** 
	* Initializing, must be called as first.
	* \param void
	* \return void
	*
	*/
	void init(int level, long baud);
	
    /**
	* Output an error message. Output message contains
	* ERROR: followed by original msg
	* Error messages are printed out, at every loglevel
	* except 0 ;-)
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/
    void error(const char* msg, ...);
	
    /**
	*Output an info message. Output message contains
	* Info messages are printed out at l
	* loglevels >= LOGLEVEL_INFOS 
	*
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

   void info(const char* msg, ...);
	
    /**
	* Output an debug message. Output message contains
	* Debug messages are printed out at l
	* loglevels >= LOGLEVEL_DEBUG
	*
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

    void debug(const char* msg, ...);
	
    /**
	* Output an verbose message. Output message contains
	* Debug messages are printed out at l
	* loglevels >= LOGLEVEL_VERBOSE
	*
	* \param msg format string to output
	* \param ... any number of variables
	* \return void
	*/

    void verbose(const char* msg, ...);   

    
private:
    void print(const char *format, va_list args);
};

extern Logging Log;

#endif