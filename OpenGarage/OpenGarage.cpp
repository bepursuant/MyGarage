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
byte  OpenGarage::alarm = 0;

static const char* config_fname = CONFIG_FNAME;
static const char* log_fname = LOG_FNAME;

/* Options name, default integer value, max value, default string value
 * Integer options don't have string value
 * String options don't have integer or max value
 */
OptionStruct OpenGarage::options[] = {
  {"fwv", OG_FWV,      255, ""},
  {"acc", OG_ACC_LOCAL,  2, ""},
  {"mnt", OG_MNT_CEILING,1, ""},
  {"dth", 50,        65535, ""},
  {"riv", 4,           300, ""},
  {"alm", OG_ALM_5,      2, ""},
  {"htp", 80,        65535, ""},
  {"mod", OG_MOD_AP,   255, ""},
  {"ssid", 0, 0, ""},  // string options have 0 max value
  {"pass", 0, 0, ""},
  {"auth", 0, 0, ""},
  {"dkey", 0, 0, DEFAULT_DKEY},
  {"name", 0, 0, DEFAULT_NAME}
};
    
void OpenGarage::begin() {
  DEBUG_PRINT("Configuring GPIO...");
  digitalWrite(PIN_RESET, HIGH);  // reset button
  pinMode(PIN_RESET, OUTPUT);
  
  digitalWrite(PIN_BUZZER, LOW);  // speaker/buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  
  digitalWrite(PIN_RELAY, LOW);   // relay
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_LED, LOW);     // status LED
  pinMode(PIN_LED, OUTPUT);
  
  digitalWrite(PIN_TRIG, HIGH);   // trigger
  pinMode(PIN_TRIG, OUTPUT);
  
  pinMode(PIN_ECHO, INPUT);       // echo
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  DEBUG_PRINTLN("ok!");
  
  state = OG_STATE_INITIAL;
  
  DEBUG_PRINT("Mounting SPIFFS...");
  if(!SPIFFS.begin()) {
    DEBUG_PRINTLN("failed!");
  } else {
    DEBUG_PRINTLN("ok!");
  }
  
  play_startup_tune();
}

void OpenGarage::options_setup() {
  int i;
  if(!SPIFFS.exists(config_fname)) { // if config file does not exist
    DEBUG_PRINT("Saving default config to SPIFFS...");
    options_save(); // save default option values
    DEBUG_PRINTLN("ok!");
    return;
  }
  options_load();
  
  if(options[OPTION_FWV].ival != OG_FWV)  {
    // if firmware version has changed
    // re-save options, thus preserving
    // shared options with previous firmwares
    options[OPTION_FWV].ival = OG_FWV;
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
  DEBUG_PRINTLN(F("ok!"));
  file.close();
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
    if(o->max)
      file.println(o->ival);
    else
      file.println(o->sval);
  }
  DEBUG_PRINTLN(F("ok!"));  
  file.close();
}

// read the distance from an ir distance sensor
// to determine the position of the door and
// whether a car is present at the time
ulong OpenGarage::read_distance_once() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  // wait till echo pin's rising edge
  while(digitalRead(PIN_ECHO)==LOW);
  unsigned long start_time = micros();
  // wait till echo pin's falling edge
  while(digitalRead(PIN_ECHO)==HIGH);
  return micros() - start_time;  
}

uint OpenGarage::read_distance() {
  byte i;
  unsigned long _time = 0;
  // average three readings to reduce noise
  byte K = 3;
  for(i=0;i<K;i++) {
    _time += read_distance_once();
    delay(50);
  }
  _time /= K;
  echo_time = _time;
  return (uint)(echo_time*0.01716f);  // 34320 cm / 2 / 10^6 s
}

bool OpenGarage::get_cloud_access_en() {
  if(options[OPTION_ACC].ival == OG_ACC_CLOUD ||
     options[OPTION_ACC].ival == OG_ACC_BOTH) {
    if(options[OPTION_AUTH].sval.length()==32) {
      return true;
    }
  }
  return false;
}

bool OpenGarage::get_local_access_en() {
  if(options[OPTION_ACC].ival == OG_ACC_LOCAL ||
     options[OPTION_ACC].ival == OG_ACC_BOTH)
     return true;
  return false;
}

void OpenGarage::write_log(const LogStruct& data) {
  DEBUG_PRINT(F("Saving log data..."));  

  File file;
  uint curr = 0;

  if(!SPIFFS.exists(log_fname)) {  // create log file

    file = SPIFFS.open(log_fname, "w");
    if(!file) {
      DEBUG_PRINTLN(F("failed to create log file!"));
      return;
    }

    // fill log file
    uint next = curr+1;
    file.write((const byte*)&next, sizeof(next));
    file.write((const byte*)&data, sizeof(LogStruct));
    LogStruct l;
    l.tstamp = 0;
    for(;next<MAX_LOG_RECORDS;next++) {
      file.write((const byte*)&l, sizeof(LogStruct));
    }
  } else {
    file = SPIFFS.open(log_fname, "r+");
    if(!file) {
      DEBUG_PRINTLN(F("failed to open log file!"));
      return;
    }
    file.readBytes((char*)&curr, sizeof(curr));
    uint next = (curr+1) % MAX_LOG_RECORDS;
    file.seek(0, SeekSet);
    file.write((const byte*)&next, sizeof(next));

    file.seek(sizeof(curr)+curr*sizeof(LogStruct), SeekSet);
    file.write((const byte*)&data, sizeof(LogStruct));
  }

  file.close();

  DEBUG_PRINTLN(F("ok!"));      
}

bool OpenGarage::read_log_start() {
  if(log_file) log_file.close();
  log_file = SPIFFS.open(log_fname, "r");
  if(!log_file) return false;
  uint curr;
  if(log_file.readBytes((char*)&curr, sizeof(curr)) != sizeof(curr)) return false;
  if(curr>=MAX_LOG_RECORDS) return false;
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

void OpenGarage::play_note(uint freq) {
  if(freq>0) {
    analogWrite(PIN_BUZZER, 512);
    analogWriteFreq(freq);
  } else {
    analogWrite(PIN_BUZZER, 0);
  }
}

#include "pitches.h"

void OpenGarage::play_startup_tune() {
  static uint melody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
  static byte duration[] = {4, 8, 8, 8};
  
  for (byte i = 0; i < sizeof(melody)/sizeof(uint); i++) {
    uint delaytime = 1000/duration[i];
    play_note(melody[i]);
    delay(delaytime);
    play_note(0);
    delay(delaytime * 0.2);    // add 30% pause between notes
  }
}