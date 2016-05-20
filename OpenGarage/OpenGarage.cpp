/* OpenGarage Firmware
 *
 * OpenGarage library
 * Mar 2016 @ OpenGarage.io
 *
 * This file is part of the OpenGarage library
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "OpenGarage.h"

ulong OpenGarage::echo_time;
byte  OpenGarage::state = OG_STATE_INITIAL;
File  OpenGarage::log_file;
uint OpenGarage::current_log_id;

static const char* config_fname = CONFIG_FNAME;
static const char* log_fname = LOG_FNAME;

/* Options name, default integer value, max value, default string value
 * Integer options don't have string value
 * String options don't have integer or max value
 */
OptionStruct OpenGarage::options[] = {
  {"firmware_version", OG_FIRMWARE_VERSION, 255, ""},
  {"sensor_type", OG_SENSOR_ULTRASONIC_CEILING, 2, ""},
  {"dth", 50, 65535, ""},
  {"read_interval", 4, 300, ""},
  {"http_port", 80, 65535, ""},
  {"mode", OG_MODE_AP, 255, ""},
  {"ssid", 0, 0, ""},  // string options have 0 max value
  {"pass", 0, 0, ""},
  {"devicekey", 0, 0, DEFAULT_DEVICEKEY},
  {"name", 0, 0, DEFAULT_NAME}
};
    
void OpenGarage::begin() {
  DEBUG_PRINT("Configuring GPIO...");
  digitalWrite(PIN_RESET, HIGH);  // reset button
  pinMode(PIN_RESET, OUTPUT);
  
  digitalWrite(PIN_RELAY, LOW);   // relay
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_LED, LOW);     // status LED
  pinMode(PIN_LED, OUTPUT);
  
  digitalWrite(PIN_TRIG, HIGH);   // trigger
  pinMode(PIN_TRIG, OUTPUT);
  
  pinMode(PIN_ECHO, INPUT);       // echo
  pinMode(PIN_CLOSED, INPUT_PULLUP); //closed sensor
  DEBUG_PRINTLN("ok!");
  
  state = OG_STATE_INITIAL;
  
  DEBUG_PRINT("Mounting SPIFFS...");
  if(!SPIFFS.begin()) {
    DEBUG_PRINTLN("failed!");
  } else {
    DEBUG_PRINTLN("ok!");
  }

}

void OpenGarage::options_setup() {
  if(!SPIFFS.exists(config_fname)) { // if config file does not exist
    DEBUG_PRINT(F("Saving default config to SPIFFS..."));
    options_save(); // save default option values
    DEBUG_PRINTLN(F("ok!"));
    return;
  }
  options_load();
  
  if(options[OPTION_FIRMWARE_VERSION].ival != OG_FIRMWARE_VERSION)  {
    // if firmware version has changed
    // re-save options, thus preserving
    // shared options with previous firmwares
    options[OPTION_FIRMWARE_VERSION].ival = OG_FIRMWARE_VERSION;
    options_save();
    return;
  }
}

void OpenGarage::options_reset() {
  DEBUG_PRINT(F("Resetting options to factory default..."));
  if(!SPIFFS.remove(config_fname)) {
    DEBUG_PRINTLN(F("failed!"));
    return;
  }
  DEBUG_PRINTLN(F("ok!"));
}

void OpenGarage::log_reset() {
  DEBUG_PRINT(F("Resetting logs to factory default..."));
  if(!SPIFFS.remove(log_fname)) {
    DEBUG_PRINTLN(F("failed!"));
    return;
  }
  DEBUG_PRINTLN(F("ok!"));  
}

int OpenGarage::find_option(String name) {
  for(byte i=0;i<NUM_OPTIONS;i++) {
    if(name == options[i].name) {
      return i;
    }
  }
  return -1;
}

void OpenGarage::options_load() {
  DEBUG_PRINT(F("Loading config file "));
  DEBUG_PRINT(config_fname);
  DEBUG_PRINT(F("..."));

  File file = SPIFFS.open(config_fname, "r");
  if(!file) {
    DEBUG_PRINTLN(F("failed!"));
    return;
  }

  while(file.available()) {
    String name = file.readStringUntil(':');
    String sval = file.readStringUntil('\n');
    sval.trim();
    int idx = find_option(name);
    if(idx<0) continue;
    if(options[idx].max) {  // this is an integer option
      options[idx].ival = sval.toInt();
    } else {  // this is a string option
      options[idx].sval = sval;
    }
  }
  file.close();

  DEBUG_PRINTLN(F("ok!"));
}

void OpenGarage::options_save() {
  DEBUG_PRINT(F("Saving config file "));
  DEBUG_PRINT(config_fname);
  DEBUG_PRINT(F("..."));

  File file = SPIFFS.open(config_fname, "w");
  if(!file) {
    DEBUG_PRINTLN(F("failed!"));
    return;
  }

  OptionStruct *o = options;
  for(byte i=0;i<NUM_OPTIONS;i++,o++) {
    file.print(o->name + ":");
    if(o->max){
      file.println(o->ival);
    }else{
      file.println(o->sval);
    }
  }
  file.close();

  DEBUG_PRINTLN(F("ok!"));  
}


uint OpenGarage::read_distance() {
  ulong distance, duration;

  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  duration = pulseIn(PIN_ECHO, HIGH, 10000);
  distance = (duration/2) / 29.1;  

  return (uint)distance;
}

void OpenGarage::write_log(const LogStruct& data) {
  DEBUG_PRINT(F("Saving log data..."));

  File file;

  if(!SPIFFS.exists(log_fname)) {

    // create a new log file, because one doesnt exist
    file = SPIFFS.open(log_fname, "w");

    if(!file) {
      DEBUG_PRINTLN(F("failed to create log file!"));
      return;
    }

    file.write((const byte*)&current_log_id, sizeof(current_log_id));
    file.write((const byte*)&data, sizeof(LogStruct));

    // and fill the rest of the file with blank (numbered) records
    LogStruct l;
    l.tstamp = 0;
    for(uint next=current_log_id;next<MAX_LOG_RECORDS;next++) {
      file.write((const byte*)&l, sizeof(LogStruct));
    }

  } else {
    // open the logfile for read +
    file = SPIFFS.open(log_fname, "r+");

    // if it doesn't open, toss a wobbly
    if(!file) {
      DEBUG_PRINTLN(F("failed to open log file!"));
      return;
    }

    // the first byte of the file is the current record ID, lets get it
    file.readBytes((char*)&current_log_id, sizeof(current_log_id));

    // create the next record ID by adding 1 and wrapping at MAX_LOG_RECORDS
    uint next = (current_log_id+1) % MAX_LOG_RECORDS;

    // seek to the beginning of the file
    file.seek(0, SeekSet);
    // write the newest record ID
    file.write((const byte*)&next, sizeof(next));

    // then navigate to this record's rightful position by calculating it's spot.
    // [size of current record ID] + ([current record ID] * [size per record])
    // and write this record as bytes for the length of a single log record
    file.seek(sizeof(current_log_id)+(current_log_id*sizeof(LogStruct)), SeekSet);
    file.write((const byte*)&data, sizeof(LogStruct));
  }

  file.close();
  DEBUG_PRINTLN(F("ok!"));      
}

bool OpenGarage::read_log_start() {
  if(log_file) log_file.close();
  log_file = SPIFFS.open(log_fname, "r");
  if(!log_file) return false;
  if(log_file.readBytes((char*)&current_log_id, sizeof(current_log_id)) != sizeof(current_log_id)) return false;
  if(current_log_id>=MAX_LOG_RECORDS) return false;
  return true;
}

bool OpenGarage::read_log(LogStruct& data, uint rec){
  if(!log_file) return false;
  log_file.seek(sizeof(current_log_id)+(rec*sizeof(LogStruct)), SeekSet);
  if(log_file.readBytes((char*)&data, sizeof(LogStruct)) != sizeof(LogStruct)) return false;
  return true;
}

bool OpenGarage::read_log_next(LogStruct& data) {
  if(!log_file) return false;
  if(log_file.readBytes((char*)&data, sizeof(LogStruct)) != sizeof(LogStruct)) return false;
  return true;  
}

bool OpenGarage::read_log_end() {
  if(!log_file) return false;
  log_file.close();
  return true;
}

bool OpenGarage::open(){
  click_relay();
}

bool OpenGarage::close(){
  click_relay();
}