#ifndef ENABLER_H
#define ENABLER_H

#include <cstring>
#include "Config.h"
#include "Types.h"
#include "ConfigSetter.h"
#include "NTPClient.h"

#define TEMP_HYSTERESIS 4
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

    void update(NTPClient& timeClient) {
      unsigned long currentTimestamp = timeClient.getEpochTime();
      int hours = timeClient.getHours();
      int minutes = timeClient.getMinutes();
      
      // Don't run this too quickly.
      if (lastUpdate == currentTimestamp) return;
      lastUpdate = currentTimestamp;
      
      //Serial.println(timeClient.getFormattedTime());
      
      // Check for updates to config
      for (int i = 0; i < numPins; i++) {
        updateStateConfig(pins[i], state[i], currentTimestamp);
      }

      // Toggles
      for (int i = 0; i < numPins; i++) {
        switch (pins[i].type) {
          case PinType_Input_TempSensorDHT11:
          case PinType_Input_TempSensorDHT22:         
            break;
          case PinType_Output_Relay:
            OutputConfig& outConf = pins[i].data.outputConfig;
            bool enabled = checkPin(pins[i], outConf, i, hours, minutes);
            if (state[i].enabled != enabled) {
              Serial.println(String("ENABLER - Pin ") + i + ", changing to: " + enabled);
              state[i].enabled = enabled;
              digitalWrite(pins[i].pinNumber, enabled ? ON : OFF);
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

    bool checkPin(Pin& p, OutputConfig& out, int i, int hours, int minutes) {
      switch(out.trigger) {
        case OutputTrigger_Temperature:
          {
            dht_data& data = pins[out.tempConfig.sensorIndex].data.tempData;
            if (!data.failed) {
              bool tempEnabled = isEnabled(out.tempConfig.temperatureTrigger, state[i].enabled, out.tempConfig.temperatureThreshold, data.temp_f);
              bool humidityEnabled = isEnabled(out.tempConfig.humidityTrigger, state[i].enabled, out.tempConfig.humidityThreshold, data.humidity);
              return tempEnabled || humidityEnabled;
            } else {
              // If the sensor failed the last reading, don't change the pin state.
              return state[i].enabled;
            }
          }
          break;
        case OutputTrigger_Schedule:
          {
            int curMinutes = hours * 60 + minutes;
            ScheduleTriggerConfig& conf = out.scheduleConfig;

            // Off at beginning and end of day.
            if (conf.startMinutes < conf.stopMinutes) {
              return conf.startMinutes <= curMinutes && curMinutes < conf.stopMinutes;
            }
            // Off in the middle of the day
            else {
              return conf.startMinutes <= curMinutes || curMinutes < conf.stopMinutes;
            }
          }
          break;
        case OutputTrigger_Manual:
          break;
        case OutputTrigger_None:
          break;
      }
      return false;
    }
    
    // Metadata used to determine if the pin needs to toggle.
    pin_meta state[NUM_PINS]; 

    // Reference to the pins.
    const int numPins;
    unsigned long lastUpdate;
    PinArray& pins;
};

#endif
