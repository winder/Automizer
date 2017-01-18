#ifndef DATA_H
#define DATA_H

String SettingsHeader = "<html><head><title>Gardenbot</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style></head><body>\n";

String SettingsFooter = "  </body></html>";

// Link to other settings pages.
String SettingsLinks = "<a href='/integrationSettings'>Integration Settings</a>\n";

// '/integrationSettings' -> '/submitIntegrationSettings'
String IntegrationFormHeader = "<h1>Integration Settings</h1><form action='/submitIntegrationSettings' method='POST'>\n";
String IntegrationFormFooter = "<input type='submit' value='Submit'></form>";

#endif
