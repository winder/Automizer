#include "ConfigSetter.h"
#include <FS.h>

String getCheckbox(String name, String description, bool checked) {
  return "<input type='checkbox' name='" + name + "'" + (checked ? " checked> ":"> ") + description + "<br>\n";
}

String getString(String name, String description, String value) {
  return description + ": <input type='text' name='" + name + "' value='" + value + "'><br>\n";
}

String getSettingsLinksBody() {
  return SettingsLinks;
}

String getIntegrationSettingsBody(const Config& c) {

  String body = String(IntegrationFormHeader);
  body += getCheckbox("useThingSpeak", "Use ThingSpeak", c.thirdPartyConfig.useThingSpeak);
  body += getString("thingSpeakKey", "ThingSpeak Key", c.thirdPartyConfig.thingSpeakKey);
  body += getString("thingSpeakChannel", "ThingSpeak Channel", String(c.thirdPartyConfig.thingSpeakChannel));
  body += "<br>\n";
  body += getCheckbox("useBlynk", "Use Blynk", c.thirdPartyConfig.useThingSpeak);
  body += getString("blynkKey", "Blynk Key", c.thirdPartyConfig.blynkKey);
  body += "<br>\n";
  body += getCheckbox("usePushingBox", "Use Pushing Box", c.thirdPartyConfig.usePushingBox);
  body += getString("pushingBoxKey", "Pushing Box Key", c.thirdPartyConfig.pushingBoxKey);
  body += "<br>\n";
  body += getCheckbox("useDweet", "Use Dweet", c.thirdPartyConfig.useDweet);
  body += getString("dweetThing", "Dweet Thing", c.thirdPartyConfig.dweetThing);
  body += String(IntegrationFormFooter);

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

