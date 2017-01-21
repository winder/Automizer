#ifndef CONFIG_SETTER_H
#define CONFIG_SETTER_H

#include "Config.h"
#include "Data.h"
#include <stdlib.h>
#include <string.h>
#include <ESP8266WebServer.h>

String getSettingsLinksBody();

String getIntegrationSettingsBody(const Config& c);

bool processIntegrationResults(ESP8266WebServer& server, Config& c);

#endif
