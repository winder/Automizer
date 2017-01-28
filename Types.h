#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <memory>

#include <stdint.h>
#define PIN_NAME_LEN 32

// For dht_data struct definition
#include "DhtReader.h"

#ifdef ESP8266
# define ON false
# define OFF true
#endif

enum PinType {
  PinType_Disabled,
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

struct ScheduleTriggerConfig {
  uint32_t start;
  uint32_t stop;
};

struct ManualTriggerConfig {
  bool enabled;
};


struct OutputConfig {
  OutputTrigger trigger;
  union {
    TemperatureTriggerConfig  tempConfig;
    ScheduleTriggerConfig     scheduleConfig;
    ManualTriggerConfig       manualConfig;
  };
};

//////////////
// Pin Data //
//////////////

// Store sensor data or output configuration.
union PinData {
  dht_data      tempData;
  OutputConfig  outputConfig;
  // 128 bytes reserved for configuration.
  char          reserved[128];
};

struct Pin {
  Pin(uint8_t num) : pinNumber(num), type(PinType_Disabled) {};

  char name[PIN_NAME_LEN];
  const uint8_t pinNumber;
  PinType type;

  PinData data;
};

#endif
