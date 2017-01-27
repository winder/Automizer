#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include <memory>

#include <stdint.h>
#define PIN_NAME_LEN 32
#include "DhtReader.h"

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

struct TemperatureTriggerConfig {
  uint32_t lower;
  uint32_t upper;
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

struct Pin {
  Pin(uint8_t num) : pinNumber(num), type(PinType_Disabled) {};

  char name[PIN_NAME_LEN];
  const uint8_t pinNumber;
  PinType type;

  // Store sensor data or output configuration.
  union {
    dht_data      tempData;
    OutputConfig  outputConfig;
    // 128 bytes reserved for configuration.
    char          reserved[128];
  };
};

#endif
