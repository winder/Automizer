#ifndef TYPES_H
#define TYPES_H

enum PinType {
  Input_TempSensor1,
  Input_TempSensor2,
  Input_TempSensor3,
  Output_Relay  
};

enum TriggerType {
  Temperature,
  Schedule,
  Manual
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

struct Pin {
  Pin(uint8_t num) : pinNumber(num) {};
  
  uint8_t pinNumber;
  PinType type;
  TriggerType trigger;
  
  union {
    TemperatureTriggerConfig tempConfig;
    ScheduleTriggerConfig scheduleConfig;
    ManualTriggerConfig manualConfig;
  };
};

#endif
