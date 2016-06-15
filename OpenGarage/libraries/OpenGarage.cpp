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

File  OpenGarage::log_file;
uint OpenGarage::current_log_id;

static const char* log_fname = LOG_FNAME;

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
    
  Log.info("Mounting SPIFFS...");
  if(!SPIFFS.begin()) {
    Log.info("failed to mount...nok!");
  } else {
    Log.info("ok!");
  }

}

void OpenGarage::log_reset() {
  Log.info("Resetting log file...");
  if(!SPIFFS.remove(log_fname)) {
    Log.info("failed to reset... nok!");
    return;
  }
  Log.info("ok!");  
}

void OpenGarage::write_log(const LogStruct& data) {
  Log.info("Saving Log {tstamp [%i], status [%i], value [%i]}...", data.tstamp, data.status, data.value);

  File file;

  if(!SPIFFS.exists(log_fname)) {

    // create a new log file, because one doesnt exist
    file = SPIFFS.open(log_fname, "w");

    if(!file) {
      Log.info("failed to create log file... nok!");
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
      Log.info("failed to open log file... nok!");
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

  Log.info("ok!"CR);
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


bool OpenGarage::open(){
  click_relay();
}

bool OpenGarage::close(){
  click_relay();
}