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
            bool enabled = pins[i].disable ? false : checkPin(pins[i], outConf, i, hours, minutes);
            if (pins[i].stale || pins[i].outputEnabled != enabled) {
              int bufSize = 6;
              char buffer[bufSize];
              snprintf(buffer, bufSize, "%02d:%02d", hours, minutes);
              Serial.println(String(buffer) + String(" - ENABLER - Pin ") + i + ", changing to: " + enabled);
              digitalWrite(pins[i].pinNumber, enabled ? config.on : config.off);
              pins[i].outputEnabled = enabled;
              pins[i].stale = false;
            }
            break;
        }
      }
    }

  private:

    bool isSensorEnabled(SensorTriggerType type, bool enabled, float threshhold, float sensorValue) {
      switch(type) {
        case SensorTriggerType_Above:
          return sensorValue > threshhold || (enabled && ((sensorValue - TEMP_HYSTERESIS) > threshhold));
        case SensorTriggerType_Below:
          return sensorValue < threshhold || (enabled && ((sensorValue + TEMP_HYSTERESIS) < threshhold));
        case SensorTriggerType_Disabled:
          return false;
      }
    }

    // hours = current day hour
    // minutes = current hour minutes
    // startMinutes = when to start the schedule each day
    // stopMinutes = when to stop the schedule each day
    bool isScheduleEnabled(int curMinutes, int startMinutes, int stopMinutes) {
      //Serial.println(String("Current minutes: ") + String(curMinutes));
      //Serial.println(String("  Start minutes: ") + String(startMinutes));
      //Serial.println(String("   Stop minutes: ") + String(stopMinutes));
      // Off at beginning and end of day.
      if (startMinutes < stopMinutes) {
        return startMinutes <= curMinutes && curMinutes < stopMinutes;
      }
      // Off in the middle of the day
      else {
        return startMinutes <= curMinutes || curMinutes < stopMinutes;
        }
    }

    bool checkPin(Pin& p, OutputConfig& out, int i, int hours, int minutes) {
      int curMinutes = hours * 60 + minutes;
      switch(out.trigger) {
        case OutputTrigger_Temperature:
          {
            dht_data& data = pins[out.tempConfig.sensorIndex].data.tempData;
            if (!data.failed) {
              bool tempEnabled = isSensorEnabled(out.tempConfig.temperatureTrigger, pins[i].outputEnabled, out.tempConfig.temperatureThreshold, data.temp_f);
              bool humidityEnabled = isSensorEnabled(out.tempConfig.humidityTrigger, pins[i].outputEnabled, out.tempConfig.humidityThreshold, data.humidity);
              return tempEnabled || humidityEnabled;
            } else {
              // If the sensor failed the last reading, don't change the pin state.
              return pins[i].outputEnabled;
            }
          }
          break;
        case OutputTrigger_Schedule:
          {
            ScheduleTriggerConfig& conf = out.scheduleConfig;
            return isScheduleEnabled(curMinutes, conf.startMinutes, conf.stopMinutes);
          }
          break;
        case  OutputTrigger_Interval:
          {
            //Serial.println(String("OutputTrigger_Interval"));
            IntervalTriggerConfig& conf = out.intervalConfig;
            bool onSchedule = isScheduleEnabled(curMinutes, conf.startMinutes, conf.stopMinutes);
            
            //Serial.println(String("curMinutes: ") + String(curMinutes));
            //Serial.println(String("conf.startMinutes: ") + String(conf.startMinutes));
            //Serial.println(String("conf.stopMinutes: ") + String(conf.stopMinutes));

            // If we're in the interval on period.
            if (onSchedule) {
              // Find out how long we've been in the on-period
              int onFor = curMinutes - conf.startMinutes;
              if (conf.startMinutes >= conf.stopMinutes) {
                if (curMinutes < conf.startMinutes) {
                  onFor = (60*24) - conf.startMinutes + curMinutes;
                } else {
                  onFor = curMinutes - conf.startMinutes;
                }
              }

              int remainder = onFor % (conf.onMinutes + conf.offMinutes);
              return remainder < conf.onMinutes;
            }
            
            return false;
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
