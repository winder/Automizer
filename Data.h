#ifndef DATA_H
#define DATA_H

#include "WString.h"

// Link to other settings pages.
static String SettingsLinks = "<a href='/integrationSettings'>Integration Settings</a>\n";

// '/integrationSettings' -> '/submitIntegrationSettings'
static String IntegrationFormHeader = "<h1>Integration Settings</h1><form action='/submitIntegrationSettings' method='POST'>\n";
static String IntegrationFormFooter = "<input type='submit' value='Submit'></form>";

#endif
