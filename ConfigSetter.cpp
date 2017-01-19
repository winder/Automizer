#include "ConfigSetter.h"

String getCheckbox(String name, String description, bool checked) {
  return "<input type='checkbox' name='" + name + "'" + (checked ? " checked> ":"> ") + description + "<br>\n";
}

String getString(String name, String description, String value) {
  return description + ": <input type='text' name='" + name + "' value='" + value + "'><br>\n";
}

String getSettingsLinksPage() {
  return SettingsHeader + SettingsLinks + SettingsFooter;
}

String getIntegrationSettingsPage(const Config& c) {

  String fields = String(IntegrationFormHeader);
  fields += getCheckbox("useThingSpeak", "Use ThingSpeak", c.thirdPartyConfig.useThingSpeak);
  fields += getString("thingSpeakKey", "ThingSpeak Key", c.thirdPartyConfig.thingSpeakKey);
  fields += getString("thingSpeakChannel", "ThingSpeak Channel", String(c.thirdPartyConfig.thingSpeakChannel));
  fields += "<br>\n";
  fields += getCheckbox("useBlynk", "Use Blynk", c.thirdPartyConfig.useThingSpeak);
  fields += getString("blynkKey", "Blynk Key", c.thirdPartyConfig.blynkKey);
  fields += "<br>\n";
  fields += getCheckbox("usePushingBox", "Use Pushing Box", c.thirdPartyConfig.usePushingBox);
  fields += getString("pushingBoxKey", "Pushing Box Key", c.thirdPartyConfig.pushingBoxKey);
  fields += "<br>\n";
  fields += getCheckbox("useDweet", "Use Dweet", c.thirdPartyConfig.useDweet);
  fields += getString("dweetThing", "Dweet Thing", c.thirdPartyConfig.dweetThing);
  fields += String(IntegrationFormFooter);

  return SettingsHeader + fields + SettingsFooter;
}

bool processIntegrationResults(ESP8266WebServer& server, Config& c) {
  c.thirdPartyConfig.useThingSpeak = strcmp(server.arg("useThingSpeak").c_str(), "") != 0;
  c.thirdPartyConfig.thingSpeakKey = server.arg("thingSpeakKey");
  String channel = server.arg("thingSpeakChannel");
  c.thirdPartyConfig.thingSpeakChannel = strtoul(channel.c_str(), NULL, 0);

  c.thirdPartyConfig.useBlynk = strcmp(server.arg("useBlynk").c_str(), "") != 0;
  c.thirdPartyConfig.blynkKey = server.arg("blynkKey");

  c.thirdPartyConfig.usePushingBox = strcmp(server.arg("usePushingBox").c_str(), "") != 0;
  c.thirdPartyConfig.pushingBoxKey = server.arg("pushingBoxKey");
  
  Serial.println(String("use dweet = ") + (c.thirdPartyConfig.useDweet ? "on" : "off"));
  c.thirdPartyConfig.useDweet = strcmp(server.arg("useDweet").c_str(), "on") == 0;
  Serial.println(String("use dweet = ") + (c.thirdPartyConfig.useDweet ? "on" : "off"));
  c.thirdPartyConfig.dweetThing = server.arg("dweetThing");
  
  if (server.args() > 0 ) {
      for ( uint8_t i = 0; i < server.args(); i++ ) {
        Serial.println(server.argName(i) + ": " + server.arg(i));
     }
  }
  return true;
}

