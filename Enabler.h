#ifndef ENABLER_H
#define ENABLER_H

#include <cstring>
#include "Config.h"
#include "Types.h"
#include "ConfigSetter.h"

#define TEMP_HYSTERESIS 2
class Enabler {
  private:
    struct pin_meta {
      // Marked if the pin configuration should be recalculated.
      bool stale = true;
      
      // Used when the time needs to changed at a future timestamp.
      unsigned long changeAt;
      
      // For hysteresis save the timestamp of the last toggle.
      unsigned long lastChange;
      
      // The current enable state
      bool enabled;
    };
    
  public:
    Enabler(PinArray& _pins, int _numPins) : pins(_pins), numPins(_numPins) {}

    void update(unsigned long currentTimestamp) {
      // Don't run this too quickly.
      if (lastUpdate == currentTimestamp) return;
      lastUpdate = currentTimestamp;
      
      // Check for updates to config
      for (int i = 0; i < numPins; i++) {
        updateStateConfig(pins[i], state[i], currentTimestamp);
      }

      // Toggles
      for (int i = 0; i < numPins; i++) {
        switch (pins[i].type) {
          case PinType_Input_TempSensorDHT11:
          case PinType_Input_TempSensorDHT22:
            //Serial.println("ENABLER");
            //Serial.println(String("F: ") + pins[i].tempData.temp_f);
            //Serial.println(String("H: ") + pins[i].tempData.humidity);            
            break;
          case PinType_Output_Relay:

            OutputConfig& outConf = pins[i].data.outputConfig;
            
            switch(outConf.trigger) {
              case OutputTrigger_Temperature:
                {
                  //Serial.println(String("Temprature Output Pin: ") + i + ", reading: " + outConf.tempConfig.sensorIndex);
                  dht_data& data = pins[outConf.tempConfig.sensorIndex].data.tempData;
                  //Serial.println(String("Sensor temp = ") + data.temp_f + ", humidity = " + data.humidity + ", failed = " + data.failed);

                  if (!data.failed) {
                    //Serial.println(String("Sensor temp = ") + data.temp_f + ", humidity = " + data.humidity + ", failed = " + data.failed);
                    //dumpPin(pins[4], 4);
                    
                    bool tempEnabled = isEnabled(outConf.tempConfig.temperatureTrigger, state[i].enabled, outConf.tempConfig.temperatureThreshold, data.temp_f);
                    bool humidityEnabled = isEnabled(outConf.tempConfig.humidityTrigger, state[i].enabled, outConf.tempConfig.humidityThreshold, data.humidity);
                    bool enabled = tempEnabled || humidityEnabled;
                    if (state[i].enabled != enabled) {
                      Serial.println(String("ENABLER Pin ") + i + ", changing to: " + enabled);
                      state[i].enabled = enabled;
                      digitalWrite(pins[i].pinNumber, enabled ? ON : OFF);
                    }
                  }
                }
                break;
              case OutputTrigger_Schedule:
                break;
              case OutputTrigger_Manual:
                break;
              case OutputTrigger_None:
                break;
            }
            break;
        }
        state[i].stale = false;
      }
    }

  private:

    void updateStateConfig(Pin& p, pin_meta& meta, int currentTimestamp) {
      if (meta.stale == false) return;
      
      // Reset data/configuration
      Serial.println(String("Pin update, stale = ") + meta.stale);
      meta.enabled = false;

      switch (p.type) {
        case PinType_Input_TempSensorDHT11:
        case PinType_Input_TempSensorDHT22:
          // Clear out sensor data for inputs
          std::memset(&(p.data), 0, sizeof(p.data));
          p.data.tempData.failed = true;
          break;
        case PinType_Output_Relay:
          meta.lastChange = 0;
          OutputConfig& outConf = p.data.outputConfig;
          switch(outConf.trigger) {
            case OutputTrigger_Schedule:
              break;
            // Don't need to pre-calculate these.
            case OutputTrigger_Temperature:
            case OutputTrigger_Manual:
            case OutputTrigger_None:
              break;
          }
          break;
      }
    }

    bool isEnabled(SensorTriggerType type, bool enabled, float threshhold, float sensorValue) {
      switch(type) {
        case SensorTriggerType_Above:
          return sensorValue > threshhold || (enabled && ((sensorValue - TEMP_HYSTERESIS) > threshhold));
        case SensorTriggerType_Below:
          return sensorValue < threshhold || (enabled && ((sensorValue + TEMP_HYSTERESIS) < threshhold));
        case SensorTriggerType_Disabled:
          return false;
      }
    }

    // Metadata used to determine if the pin needs to toggle.
    pin_meta state[NUM_PINS]; 

    // Reference to the pins.
    const int numPins;
    unsigned long lastUpdate;
    PinArray& pins;
};

#endif
