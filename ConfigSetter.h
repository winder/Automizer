#ifndef CONFIG_SETTER_H
#define CONFIG_SETTER_H

#include "Config.h"
#include <stdlib.h>
#include <string.h>
#include <ESP8266WebServer.h>

String getSettingsLinksBody();
String getPinSettingsBody(const Config& c);

String getIntegrationSettingsBody(const Config& c);

bool processIntegrationResults(ESP8266WebServer& server, Config& c);
bool processPinResults(ESP8266WebServer& server, Config& c);
bool processPinJsonResults(ESP8266WebServer& server, Config& c);

void loadConfig(Config& c);

void saveConfig(Config& c);

#endif
