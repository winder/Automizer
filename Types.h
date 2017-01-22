#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#define PIN_NAME_LEN 32

enum PinType {
  Disabled,
  Input_TempSensorDHT11,
  Input_TempSensorDHT22,
  Output_Relay  
};

enum TriggerType {
  None,
  Temperature,
  Schedule,
  Manual,
};

struct InputConfig {
  
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
  TriggerType trigger;
  union {
    TemperatureTriggerConfig  tempConfig;
    ScheduleTriggerConfig     scheduleConfig;
    ManualTriggerConfig       manualConfig;
  };
};

struct Pin {
  Pin(uint8_t num) : pinNumber(num), type(Disabled) {};
  
  char name[PIN_NAME_LEN];
  const uint8_t pinNumber;
  PinType type;

  union {
    InputConfig   inputConfig;
    OutputConfig  outputConfig;
    // 128 bytes reserved for configuration.
    char          reserved[128];
  };
};

#endif
