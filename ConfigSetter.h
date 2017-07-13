#ifndef CONFIG_SETTER_H
#define CONFIG_SETTER_H

#include "Config.h"
#include <stdlib.h>
#include <string.h>
#include <ESP8266WebServer.h>

String getSettingsLinksBody();
String getPinSettingsBody(const Config& c);
String getGlobalSettingsBody(const Config& c);

String getIntegrationSettingsBody(const Config& c);

bool processIntegrationResults(ESP8266WebServer& server, Config& c);
bool processPinResults(ESP8266WebServer& server, Config& c);
bool processPinJsonResults(ESP8266WebServer& server, Config& c);
bool processGlobalSettingsResults(ESP8266WebServer& server, Config& c);

bool loadConfig(Config& c);
bool saveConfig(Config& c);

// Given a JSON string, update the config file.
bool loadJsonConfig(const char* s, Config& c);
// Convert config file to JSON
bool configToJson(Config& c, char* json, size_t maxSize, bool pinsOnly=false);

// Pin object helpers...
String pinTypeToString(PinType type);
PinType getTypeFromString(String& s);

String outputTriggerToString(OutputTrigger trigger);
OutputTrigger getOutputTriggerFromString(String& s);

String sensorTriggerTypeToString(SensorTriggerType type);
SensorTriggerType getSensorTriggerTypeFromString(String& s);

void dumpPin(Pin& p, int idx);

#endif
