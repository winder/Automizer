#ifndef DATA_H
#define DATA_H

#include "WString.h"

static String SettingsHeader = "<html><head><title>Gardenbot</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head><body>\n";

static String SettingsFooter = "  </body></html>";

// Link to other settings pages.
static String SettingsLinks = "<a href='/integrationSettings'>Integration Settings</a>\n";

// '/integrationSettings' -> '/submitIntegrationSettings'
static String IntegrationFormHeader = "<h1>Integration Settings</h1><form action='/submitIntegrationSettings' method='POST'>\n";
static String IntegrationFormFooter = "<input type='submit' value='Submit'></form>";

#endif
