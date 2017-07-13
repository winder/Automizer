#include "ConfigSetter.h"
#include <FS.h>
#include <cstring>
#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

String getCheckbox(String name, String description, bool checked) {
  return "<input type='checkbox' name='" + name + "'" + (checked ? " checked> ":"> ") + description + "\n";
}

String getString(String name, String description, String value) {
  return description + ": <input type='text' name='" + name + "' value='" + value + "'>\n";
}

String getNumber(String name, String description, String value) {
  return description + ": <input type='number' name='" + name + "' value='" + value + "'>\n";
}

String getSettingsLinksBody() {
  return "<a href='/settings'>Settings</a>\n<br><a href='/integrationSettings'>Integration Settings</a>\n<br>\n<a href='/pinSettings'>Pin Settings</a>";
}

PinType stringToPinType(String t) {
  if (t == "disabled") return PinType_Disabled;
  if (t == "dht11")    return PinType_Input_TempSensorDHT11;
  if (t == "dht22")    return PinType_Input_TempSensorDHT22;
  if (t == "relay")    return PinType_Output_Relay;
  return PinType_Disabled;
}

PinType getTypeFromString(String& s) {
  if (s == "disabled") return PinType_Disabled;
  if (s == "dht11")    return PinType_Input_TempSensorDHT11;
  if (s == "dht22")    return PinType_Input_TempSensorDHT22;
  if (s == "relay")    return PinType_Output_Relay;
  return PinType_Disabled;
}

bool processPinJsonResults(ESP8266WebServer& server, Config& c) {
  if (server.hasArg("plain")) {
    Serial.println(String("Data: ") + server.arg("plain").c_str());
    if (loadJsonConfig(server.arg("plain").c_str(), c)) {
      Serial.println("GOOD!");
    } else {
      Serial.println("BAD!");
    }
  }
}

// Generate HTML form for global settings
String getGlobalSettingsBody(const Config& c) {
  String body = "<form action='/submitSettings' method='POST'>";

  body += getString(String("timeZoneOffsetMinutes"), String("Time Zone Offset (minutes)"), String(c.timeZoneOffsetMinutes)) + "<br>\n";
  body += getString(String("updateInterval"), String("Update Interval (seconds)"), String(c.updateInterval)) + "<br>\n";
  body += getString(String("uploadInterval"), String("Upload Interval (seconds)"), String(c.uploadInterval)) + "<br>\n";

  body += "<input type='submit' value='Submit'></form>";

  return body;
}

bool processGlobalSettingsResults(ESP8266WebServer& server, Config& c) {
  String timeZoneOffsetMinutes = server.arg("timeZoneOffsetMinutes");
  String updateInterval = server.arg("updateInterval");
  String uploadInterval = server.arg("uploadInterval");

  c.timeZoneOffsetMinutes = timeZoneOffsetMinutes.toInt();
  c.updateInterval = updateInterval.toInt();
  c.uploadInterval = uploadInterval.toInt();

  c.configInitialized = false;
}


// Generate HTML form for integration settings
String getIntegrationSettingsBody(const Config& c) {
  String body = "<h1>Integration Settings</h1><form action='/submitIntegrationSettings' method='POST'>\n";
  body += getCheckbox("useThingSpeak", "Use ThingSpeak", c.thirdPartyConfig.useThingSpeak) + "<br>\n";
  body += getString("thingSpeakKey", "ThingSpeak Key", c.thirdPartyConfig.thingSpeakKey) + "<br>\n";
  body += getString("thingSpeakChannel", "ThingSpeak Channel", String(c.thirdPartyConfig.thingSpeakChannel)) + "<br>\n";
  body += "<br>\n";
  body += getCheckbox("useBlynk", "Use Blynk", c.thirdPartyConfig.useThingSpeak) + "<br>\n";
  body += getString("blynkKey", "Blynk Key", c.thirdPartyConfig.blynkKey) + "<br>\n";
  body += "<br>\n";
  body += getCheckbox("usePushingBox", "Use Pushing Box", c.thirdPartyConfig.usePushingBox) + "<br>\n";
  body += getString("pushingBoxKey", "Pushing Box Key", c.thirdPartyConfig.pushingBoxKey) + "<br>\n";
  body += "<br>\n";
  body += getCheckbox("useDweet", "Use Dweet", c.thirdPartyConfig.useDweet) + "<br>\n";
  body += getString("dweetThing", "Dweet Thing", c.thirdPartyConfig.dweetThing) + "<br>\n";
  body += "<input type='submit' value='Submit'></form>";

  return body;
}

// Process form results and save them to config
bool processIntegrationResults(ESP8266WebServer& server, Config& c) {
  // Process thingspeak
  c.thirdPartyConfig.useThingSpeak = strcmp(server.arg("useThingSpeak").c_str(), "") != 0;
  strncpy(c.thirdPartyConfig.thingSpeakKey, server.arg("thingSpeakKey").c_str(), KEY_LEN);
  String channel = server.arg("thingSpeakChannel");
  c.thirdPartyConfig.thingSpeakChannel = strtoul(channel.c_str(), NULL, 0);

  // Process blynk
  c.thirdPartyConfig.useBlynk = strcmp(server.arg("useBlynk").c_str(), "") != 0;
  strncpy(c.thirdPartyConfig.blynkKey, server.arg("blynkKey").c_str(), KEY_LEN);

  // Process pushing box
  c.thirdPartyConfig.usePushingBox = strcmp(server.arg("usePushingBox").c_str(), "") != 0;
  strncpy(c.thirdPartyConfig.pushingBoxKey, server.arg("pushingBoxKey").c_str(), KEY_LEN);

  // Process dweet
  c.thirdPartyConfig.useDweet = strcmp(server.arg("useDweet").c_str(), "on") == 0;
  strncpy(c.thirdPartyConfig.dweetThing, server.arg("dweetThing").c_str(), KEY_LEN);
  
  if (server.args() > 0 ) {
      for ( uint8_t i = 0; i < server.args(); i++ ) {
        Serial.println(server.argName(i) + ": " + server.arg(i));
     }
  }
  return true;
}


bool saveConfig(Config& c) {
  File configFile = SPIFFS.open("/config.conf", "w+");

  if (!configFile)
  {
    Serial.println(F("Failed to open config.conf"));
    return false;
  }

  // Serialize json and write to file.
  int maxSize = sizeof(Config) * 2;
  std::vector<char> buffer;
  buffer.reserve(maxSize);
  if(configToJson(c, &buffer[0], maxSize)) {
    unsigned char* data = reinterpret_cast<unsigned char*>(&buffer[0]);
    size_t bytes = configFile.write(data, maxSize);
    Serial.printf("END Position =%u \n", configFile.position());
    configFile.close();
  } else {
    Serial.println("Failed to save config.");
  }

  /*
    // Serialize struct to file.
    Serial.println(F("Opened config.conf for UPDATE...."));
    Serial.printf("Start Position =%u \n", configFile.position());
  
    unsigned char * data = reinterpret_cast<unsigned char*>(&c); // use unsigned char, as uint8_t is not guarunteed to be same width as char...
    size_t bytes = configFile.write(data, sizeof(Config)); // C++ way
  
    Serial.printf("END Position =%u \n", configFile.position());
    configFile.close();
  */
}

bool loadConfig(Config& c) {
  File configFile = SPIFFS.open("/config.conf", "r");
  
  if (!configFile)
  {
    Serial.println(F("Failed to open config.conf"));
    return false;
  }

  // Read file.
  int val;
  char ch;
  std::vector<char> buffer;
  buffer.reserve(2048);
  while ((val = configFile.read()) != -1) {
    buffer.push_back(char(val));
  }

  // Process json.
  bool success = loadJsonConfig(&buffer[0], c);
  configFile.close();
  return success;
  /*
  // Serialize struct from file.
  Serial.println(F("Opened config.conf"));
  Serial.print(F("CONFIG FILE CONTENTS----------------------"));
  Serial.println();

  uint8_t* config = reinterpret_cast<unsigned char*>(&c);
  size_t size = configFile.size();
  uint8_t counter = 0; 

  for(int j=0;j<sizeof(Config);j++){
    config[j] = configFile.read();
  }

  configFile.close();
  */
}


bool loadJsonConfig(const char* s, Config& c) {
  DynamicJsonBuffer jsonBuffer;

  Serial.println("load json config...");
  Serial.println(s);
  JsonObject& root = jsonBuffer.parseObject(s);
  Serial.println("parsed!");
  // Root config
  if (root.containsKey("config")) {
    JsonObject& conf = root["config"].asObject();
    
    c.ledPin                = conf["led_pin"];
    c.minSensorIntervalMs   = conf["minSensorIntervalMs"];
    c.timeZoneOffsetMinutes = conf["timeZoneOffsetMinutes"];
    c.updateInterval        = conf["updateInterval"];
    c.uploadInterval        = conf["uploadInterval"];
  }

  // Third party config
  if (root.containsKey("third_party")) {
    JsonObject& thirdParty = root["third_party"].asObject();
    ThirdPartyConfig& tpc = c.thirdPartyConfig;
    
    tpc.useThingSpeak = thirdParty["useThingSpeak"].as<bool>();
    tpc.thingSpeakChannel = thirdParty["thingSpeakChannel"].as<unsigned long>();
    strncpy(tpc.thingSpeakKey, thirdParty["thingSpeakKey"], KEY_LEN);
    
    tpc.useBlynk = thirdParty["useBlynk"].as<bool>();
    strncpy(tpc.blynkKey, thirdParty["blynkKey"], KEY_LEN);
    
    tpc.usePushingBox = thirdParty["usePushingBox"].as<bool>();
    strncpy(tpc.pushingBoxKey, thirdParty["pushingBoxKey"], KEY_LEN);
    
    tpc.useDweet = thirdParty["useDweet"].as<bool>();
    strncpy(tpc.dweetThing, thirdParty["dweetThing"], KEY_LEN);
  }

  // Load pins if it exists.
  if (root.containsKey("pins")) {
    JsonArray& nestedArray = root["pins"].asArray();
    for (JsonObject& pinObject : nestedArray){
      Pin& p = c.pins[pinObject["pin_idx"].as<int>()];

      if (pinObject.containsKey("pin_number")) {
        //p.pinNumber = pinObject["pin_number"];
      }
      
      if (pinObject.containsKey("name")) {
        strncpy(p.name, pinObject["name"], PIN_NAME_LEN);
      }

      if (pinObject.containsKey("type")) {
        String type(pinObject["type"].asString());
        p.type = getTypeFromString(type);
        
        switch (p.type) {
          case PinType_Input_TempSensorDHT11:
          case PinType_Input_TempSensorDHT22:
            // Nothing else.
            break;
          case PinType_Output_Relay:
            String outputTrigger(pinObject["trigger"].asString());
            outputTrigger.toLowerCase();
            p.data.outputConfig.trigger = getOutputTriggerFromString(outputTrigger);
            switch(p.data.outputConfig.trigger) {
              case OutputTrigger_None:
                break;
              case OutputTrigger_Schedule:
                {
                  int hours, minutes;
                  
                  String start(pinObject["trigger_schedule_start"].asString());
                  hours   = start.substring(0,2).toInt();
                  minutes = start.substring(3,5).toInt();
                  p.data.outputConfig.scheduleConfig.startMinutes = hours * 60 + minutes;
                  //Serial.println(start + ", h: " + hours + ", m: " + minutes + ", store: " + (hours * 60 + minutes));
                  
                  String stop(pinObject["trigger_schedule_stop"].asString());
                  hours   = stop.substring(0,2).toInt();
                  minutes = stop.substring(3,5).toInt();
                  p.data.outputConfig.scheduleConfig.stopMinutes = hours * 60 + minutes;
                  //Serial.println(stop + ", h: " + hours + ", m: " + minutes + ", store: " + (hours * 60 + minutes));
                }
                break;
              case OutputTrigger_Interval:
                {
                  int hours, minutes;
                  
                  String start(pinObject["trigger_interval_start"].asString());
                  hours   = start.substring(0,2).toInt();
                  minutes = start.substring(3,5).toInt();
                  p.data.outputConfig.intervalConfig.startMinutes = hours * 60 + minutes;
                  //Serial.println(start + ", h: " + hours + ", m: " + minutes + ", store: " + (hours * 60 + minutes));
                  
                  String stop(pinObject["trigger_interval_stop"].asString());
                  hours   = stop.substring(0,2).toInt();
                  minutes = stop.substring(3,5).toInt();
                  p.data.outputConfig.intervalConfig.stopMinutes = hours * 60 + minutes;
                  //Serial.println(stop + ", h: " + hours + ", m: " + minutes + ", store: " + (hours * 60 + minutes));

                  String on(pinObject["trigger_interval_on"].asString());
                  p.data.outputConfig.intervalConfig.onMinutes = on.toInt();
                  
                  String off(pinObject["trigger_interval_off"].asString());
                  p.data.outputConfig.intervalConfig.offMinutes = off.toInt();
                }
                break;
              case OutputTrigger_Manual:
                break;
              case OutputTrigger_Temperature:
                {
                  p.data.outputConfig.tempConfig.sensorIndex          = pinObject["trigger_sensor_pin"];
                  String tt(pinObject["trigger_temperature_event"].asString());
                  tt.toLowerCase();
                  p.data.outputConfig.tempConfig.temperatureTrigger   = getSensorTriggerTypeFromString(tt);
                  p.data.outputConfig.tempConfig.temperatureThreshold = pinObject["trigger_temperature_f"];
                  String ht(pinObject["trigger_humidity_event"].asString());
                  ht.toLowerCase();
                  p.data.outputConfig.tempConfig.humidityTrigger      = getSensorTriggerTypeFromString(ht);
                  p.data.outputConfig.tempConfig.humidityThreshold    = pinObject["trigger_humidity_percent"];
                }
                break;
            }
            break;
        }
      }
    }
  }

  return true;
}

bool configToJson(Config& c, char* json, size_t maxSize, bool pinsOnly) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& data = root.createNestedArray("pins");

  // Root config
  if (!pinsOnly) {
    JsonObject& config = root.createNestedObject("config");
    config["led_pin"] = c.ledPin;
    config["minSensorIntervalMs"] = c.minSensorIntervalMs;
    config["timeZoneOffsetMinutes"] = c.timeZoneOffsetMinutes;
    config["updateInterval"] = c.updateInterval;
    config["uploadInterval"] = c.uploadInterval;
  }

  // 3rd Party
  if (!pinsOnly) {
    ThirdPartyConfig& tpc = c.thirdPartyConfig;
    JsonObject& thirdParty = root.createNestedObject("third_party");
    
    thirdParty["useThingSpeak"] = tpc.useThingSpeak;
    thirdParty["thingSpeakChannel"] = tpc.thingSpeakChannel;
    thirdParty["thingSpeakKey"] = tpc.thingSpeakKey;
  
    thirdParty["useBlynk"] = tpc.useBlynk;
    thirdParty["blynkKey"] = tpc.blynkKey;
    
    thirdParty["usePushingBox"] = tpc.usePushingBox;
    thirdParty["pushingBoxKey"] = tpc.pushingBoxKey;
    
    thirdParty["useDweet"] = tpc.useDweet;
    thirdParty["dweetThing"] = tpc.dweetThing;
  }
  
  // Pins
  for (int i = 0; i < NUM_PINS; i++) {
    Pin& p = c.pins[i];
    JsonObject& pinObject = data.createNestedObject();
    pinObject["pin_idx"] = i;
    pinObject["pin_number"] = p.pinNumber;
    pinObject["name"] = p.name;
    pinObject["type"] = pinTypeToString(p.type);

    switch (p.type) {
      case PinType_Input_TempSensorDHT11:
      case PinType_Input_TempSensorDHT22:
        // Nothing else.
        break;
      case PinType_Output_Relay:
        pinObject["trigger"] = outputTriggerToString(p.data.outputConfig.trigger);
        switch(p.data.outputConfig.trigger) {
          case OutputTrigger_None:
            break;
          case OutputTrigger_Schedule:
            {
              int bufSize = 6;
              char buffer[bufSize];
              
              int startMinutes = p.data.outputConfig.scheduleConfig.startMinutes;
              snprintf(buffer, bufSize, "%02d:%02d", startMinutes/60, startMinutes%60);
              pinObject["trigger_schedule_start"] = String(buffer);
              
              int stopMinutes = p.data.outputConfig.scheduleConfig.stopMinutes;
              snprintf(buffer, bufSize, "%02d:%02d", stopMinutes/60, stopMinutes%60);
              pinObject["trigger_schedule_stop"]  = String(buffer);
            }
            break;
          case OutputTrigger_Interval:
            {
              int bufSize = 6;
              char buffer[bufSize];
              
              int startMinutes = p.data.outputConfig.intervalConfig.startMinutes;
              snprintf(buffer, bufSize, "%02d:%02d", startMinutes/60, startMinutes%60);
              pinObject["trigger_interval_start"] = String(buffer);
              
              int stopMinutes = p.data.outputConfig.intervalConfig.stopMinutes;
              snprintf(buffer, bufSize, "%02d:%02d", stopMinutes/60, stopMinutes%60);
              pinObject["trigger_interval_stop"]  = String(buffer);

              int onMinutes = p.data.outputConfig.intervalConfig.onMinutes;
              pinObject["trigger_interval_on"] = String(onMinutes);
              
              int offMinutes = p.data.outputConfig.intervalConfig.offMinutes;
              pinObject["trigger_interval_off"] = String(offMinutes);
            }
            break;
          case OutputTrigger_Manual:
            break;
          case OutputTrigger_Temperature:
            pinObject["trigger_sensor_pin"] =          p.data.outputConfig.tempConfig.sensorIndex;
            pinObject["trigger_temperature_event"] =   sensorTriggerTypeToString(p.data.outputConfig.tempConfig.temperatureTrigger);
            pinObject["trigger_temperature_f"] =       p.data.outputConfig.tempConfig.temperatureThreshold;
            pinObject["trigger_humidity_event"] =      sensorTriggerTypeToString(p.data.outputConfig.tempConfig.humidityTrigger);
            pinObject["trigger_humidity_percent"] =    p.data.outputConfig.tempConfig.humidityThreshold;
            break;
        }
        break;
    }
  }

  Serial.println(String("Json Buffer Size: ") + jsonBuffer.size());
  if (jsonBuffer.size() > maxSize) return false;
  root.printTo(json, maxSize);
  return true;
}
    
String pinTypeToString(PinType type) {
  switch(type) {
    case PinType_Disabled:
      return "disabled";
    case PinType_Input_TempSensorDHT11:
      return "dht11";
    case PinType_Input_TempSensorDHT22:
      return "dht22";
    case PinType_Output_Relay:
      return "relay";
  }
  return "";
}

OutputTrigger getOutputTriggerFromString(String& s) {
  if (s == "none")        return OutputTrigger_None;
  if (s == "environment") return OutputTrigger_Temperature;
  if (s == "schedule")    return OutputTrigger_Schedule;
  if (s == "interval")    return OutputTrigger_Interval;
  if (s == "manual")      return OutputTrigger_Manual;
  return OutputTrigger_None;
}

String outputTriggerToString(OutputTrigger trigger) {
  switch(trigger) {
    case OutputTrigger_None:        return "none";
    case OutputTrigger_Temperature: return "environment";
    case OutputTrigger_Schedule:    return "schedule";
    case OutputTrigger_Interval:    return "interval";
    case OutputTrigger_Manual:      return "manual";
  }
  return "";
}

SensorTriggerType getSensorTriggerTypeFromString(String& s) {
  if (s == "disabled") return SensorTriggerType_Disabled;
  if (s == "above")    return SensorTriggerType_Above;
  if (s == "below")    return SensorTriggerType_Below;
  return SensorTriggerType_Disabled;
}

String sensorTriggerTypeToString(SensorTriggerType type) {
  switch(type) {
    case SensorTriggerType_Disabled:  return "disabled";
    case SensorTriggerType_Above:     return "above";
    case SensorTriggerType_Below:     return "below";
  }
  return "";
}

void dumpPin(Pin& p, int idx) {
  Serial.println("-----------------------------");
  Serial.println(String("Pin ") + idx);
  Serial.println(String("Name:    ") + p.name);
  Serial.println(String("Number:  ") + p.pinNumber);
  Serial.println(String("Type:    ") + pinTypeToString(p.type));
  
  switch(p.type) {
    case PinType_Input_TempSensorDHT11:
      Serial.println(String("  F: ") + p.data.tempData.temp_f + ", H: " + p.data.tempData.humidity);
      break;
    case PinType_Input_TempSensorDHT22:
      Serial.println(String("  F: ") + p.data.tempData.temp_f + ", H: " + p.data.tempData.humidity);
      break;
    case PinType_Output_Relay:
      Serial.println(String("Trigger: ") + outputTriggerToString(p.data.outputConfig.trigger));
      switch(p.data.outputConfig.trigger) {
        case OutputTrigger_None:
          break;
        case OutputTrigger_Schedule:
          Serial.println(String("Start minutes: ") + p.data.outputConfig.scheduleConfig.startMinutes);
          Serial.println(String("Stop minutes:  ") + p.data.outputConfig.scheduleConfig.stopMinutes);
          break;
        case OutputTrigger_Interval:
          Serial.println(String("Start minutes: ") + p.data.outputConfig.intervalConfig.startMinutes);
          Serial.println(String("Stop minutes:  ") + p.data.outputConfig.intervalConfig.stopMinutes);
          Serial.println(String("On minutes:    ") + p.data.outputConfig.intervalConfig.onMinutes);
          Serial.println(String("Off minutes:   ") + p.data.outputConfig.intervalConfig.offMinutes);
          break;
        case OutputTrigger_Manual:
          break;
        case OutputTrigger_Temperature:
          Serial.println(String("Sensor idx:       ") + p.data.outputConfig.tempConfig.sensorIndex);
          Serial.println(String("Temp trigger:     ") + sensorTriggerTypeToString(p.data.outputConfig.tempConfig.temperatureTrigger));
          Serial.println(String("Temp thresh:      ") + p.data.outputConfig.tempConfig.temperatureThreshold);
          Serial.println(String("Humidity trigger: ") + sensorTriggerTypeToString(p.data.outputConfig.tempConfig.humidityTrigger));
          Serial.println(String("Humidity thresh:  ") + p.data.outputConfig.tempConfig.humidityThreshold);
          break;
      }
      break;
    case PinType_Disabled:
      Serial.println("Disabled");
  }
  Serial.println("-----------------------------");
}
