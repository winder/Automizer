#include "ConfigSetter.h"
#include <FS.h>
#include <cstring>

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
  return "<a href='/integrationSettings'>Integration Settings</a>\n<br>\n<a href='/pinSettings'>Pin Settings</a>";
}

String getEnumName(PinType type) {
  switch(type) {
    default:
    case Disabled:              return "Disabled";
    case Input_TempSensorDHT11: return "Temperature Input (DHT11)";
    case Input_TempSensorDHT22: return "Temperature Input (DHT22)";
    case Output_Relay:          return "Relay";                    
  }
  return "Disabled";
}

String getPinCombo(String name, String description, PinType selected) {
  String field = description + ": ";
  field += "<select name='" + name + "'>\n";
  field += "  <option value='disabled'" + String(((selected == Disabled) ? " selected>":">")) + getEnumName(Disabled) + "</option>\n";
  field += "  <option value='dht11'" + String(((selected == Input_TempSensorDHT11) ? " selected>":">")) + getEnumName(Input_TempSensorDHT11) + "</option>\n";
  field += "  <option value='dht22'" + String(((selected == Input_TempSensorDHT22) ? " selected>":">")) + getEnumName(Input_TempSensorDHT22) + "</option>\n";
  field += "  <option value='relay'" + String(((selected == Output_Relay) ? " selected>":">")) + getEnumName(Output_Relay) + "</option>\n";
  field += "</select>\n";
  return field;
}

String getPinSettingsBody(const Config& c) {
  String body = "<form action='/submitPinSettings' method='POST'>";
  
  for (int i = 0; i < NUM_PINS; i++) {
    String pinName = String("D") + (i+1);
    body += "<br>Pin " + pinName + " Settings<br>\n";
    body += getString(String(i) + "_name", String("Name"), String(c.pins[i].name)) + "<br>\n";
    body += getPinCombo(String(i), String("Type"), c.pins[i].type) + "<br>\n";
  }

  body += "<input type='submit' value='Submit'></form>";

  return body;
}

PinType stringToPinType(String t) {
  if (t == "disabled") return Disabled;
  if (t == "dht11")    return Input_TempSensorDHT11;
  if (t == "dht22")    return Input_TempSensorDHT22;
  if (t == "relay")    return Output_Relay;
  return Disabled;
}

bool processPinResults(ESP8266WebServer& server, Config& c) {
  bool typeChanged[NUM_PINS];

  // First pass, get global settings and save changed flag.
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    int idx = server.argName(i).charAt(0) - '0';
    if (idx < NUM_PINS && idx > -1) {
      //Pin& p = &(c.pins[idx]);
      
      // pin type
      if (server.argName(i).length() == 1) {
        PinType newType = stringToPinType(server.arg(i));
        if (c.pins[idx].type != newType) {
          c.pins[idx].type = newType;
          std::memset(c.pins[idx].reserved, 0, sizeof c.pins[idx].reserved);
        }
      }
      // pin name
      if (server.argName(i).endsWith("_name")) {
        strncpy(c.pins[idx].name, server.arg(i).c_str(), PIN_NAME_LEN);
      }
    }
  }

  // Second pass, pinType advanced settings knowing that the type is now set.
}

bool processPinJsonResults(ESP8266WebServer& server, Config& c) {
  if (server.args() > 0 ) {
      for ( uint8_t i = 0; i < server.args(); i++ ) {
        Serial.println(server.argName(i) + ": " + server.arg(i));
     }
  }

  if (server.hasArg("plain")) {
    Serial.println("Plain: \n" + server.arg("plain"));
  }
}

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


void saveConfig(Config& c) {
  File configFile = SPIFFS.open("/config.conf", "w+");
        
  if (!configFile)
  {
    Serial.println(F("Failed to open config.conf"));            
  } else {
    Serial.println(F("Opened config.conf for UPDATE...."));
    Serial.printf("Start Position =%u \n", configFile.position());
  
    unsigned char * data = reinterpret_cast<unsigned char*>(&c); // use unsigned char, as uint8_t is not guarunteed to be same width as char...
    size_t bytes = configFile.write(data, sizeof(Config)); // C++ way
  
    Serial.printf("END Position =%u \n", configFile.position());
    configFile.close();
  }
}


void loadConfig(Config& c) {
  File configFile = SPIFFS.open("/config.conf", "r");
  
  if (!configFile)
  {
    Serial.println(F("Failed to open config.conf")); 
  } else {
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
  }
}

