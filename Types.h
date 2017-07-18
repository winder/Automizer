#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <memory>
#include <ArduinoJson.h>

#include <stdint.h>
#define PIN_NAME_LEN 32

// For dht_data struct definition
#include "DhtReader.h"

#ifdef ESP8266
# define ON false
# define OFF true
#endif

enum PinType {
  PinType_None,
  PinType_Input_TempSensorDHT11,
  PinType_Input_TempSensorDHT22,
  PinType_Output_Relay  
};

////////////
// Output //
////////////

enum OutputTrigger {
  OutputTrigger_None,
  OutputTrigger_Temperature,
  OutputTrigger_Schedule,
  OutputTrigger_Interval,
  OutputTrigger_Manual,
};

enum SensorTriggerType {
  SensorTriggerType_Disabled,
  SensorTriggerType_Above,
  SensorTriggerType_Below
};

struct TemperatureTriggerConfig {
  // Which DHT11/DHT22 input sensor to get data from.
  uint8_t sensorIndex;
  
  SensorTriggerType temperatureTrigger;
  uint32_t temperatureThreshold;
  
  SensorTriggerType humidityTrigger;
  uint32_t humidityThreshold;
};

// Minutes can be between 1 and 24 * 60 (number of minutes in a day)
struct ScheduleTriggerConfig {
  uint16_t startMinutes;
  uint16_t stopMinutes;
};

struct ManualTriggerConfig {
  bool enabled;
};

// Intervals will run 
struct IntervalTriggerConfig {
  uint16_t startMinutes;
  uint16_t stopMinutes;
  uint16_t onMinutes;
  uint16_t offMinutes;
};

struct OutputConfig {
  OutputTrigger trigger;
  union {
    TemperatureTriggerConfig  tempConfig;
    ScheduleTriggerConfig     scheduleConfig;
    ManualTriggerConfig       manualConfig;
    IntervalTriggerConfig     intervalConfig;
  };
};

//////////////
// Pin Data //
//////////////

// Store sensor data or output configuration.
union PinData {
  dht_data      tempData;
  OutputConfig  outputConfig;
};

struct Pin {
  Pin(uint8_t num) : Pin(num, true) {};
  Pin(uint8_t num, boolean outputAllowed) : outputAllowed(outputAllowed), pinNumber(num), type(PinType_None) {};

  bool disable = false;
  bool stale = true;
  bool outputAllowed = true;
  bool outputEnabled = false;
  char name[PIN_NAME_LEN];
  const uint8_t pinNumber;
  PinType type;
  PinData data;
};

#endif
