#ifndef ENABLER_H
#define ENABLER_H

#include <cstring>
#include "Config.h"
#include "Types.h"
#include "ConfigSetter.h"
#include "NTPClient.h"

#define TEMP_HYSTERESIS 4
class Enabler {

  public:
    Enabler(PinArray& _pins, int _numPins, Config& conf) : pins(_pins), numPins(_numPins), config(conf) {}

    void update(NTPClient& timeClient) {
      unsigned long currentTimestamp = timeClient.getEpochTime();
      int hours = timeClient.getHours();
      int minutes = timeClient.getMinutes();
      
      // Don't run this too quickly.
      if (lastUpdate == currentTimestamp) return;
      lastUpdate = currentTimestamp;
      
      //Serial.println(timeClient.getFormattedTime());
      
      // Toggles
      for (int i = 0; i < numPins; i++) {
        switch (pins[i].type) {
          case PinType_Input_TempSensorDHT11:
          case PinType_Input_TempSensorDHT22:         
            break;
          case PinType_Output_Relay:
            OutputConfig& outConf = pins[i].data.outputConfig;
            bool enabled = checkPin(pins[i], outConf, i, hours, minutes);
            if (pins[i].stale || pins[i].enabled != enabled) {
              Serial.println(String("ENABLER - Pin ") + i + ", changing to: " + enabled);
              digitalWrite(pins[i].pinNumber, enabled ? config.on : config.off);
              pins[i].enabled = enabled;
              pins[i].stale = false;
            }
            break;
        }
      }
    }

  private:

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
              bool tempEnabled = isEnabled(out.tempConfig.temperatureTrigger, pins[i].enabled, out.tempConfig.temperatureThreshold, data.temp_f);
              bool humidityEnabled = isEnabled(out.tempConfig.humidityTrigger, pins[i].enabled, out.tempConfig.humidityThreshold, data.humidity);
              return tempEnabled || humidityEnabled;
            } else {
              // If the sensor failed the last reading, don't change the pin state.
              return pins[i].enabled;
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

    // Reference to the pins.
    const int numPins;
    unsigned long lastUpdate;
    PinArray& pins;
    Config& config;
};

#endif
