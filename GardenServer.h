#ifndef GARDEN_SERVER_H
#define GARDEN_SERVER_H

#include "Config.h"
#include "DhtReader.h"
#include <ESP8266WebServer.h>

class GardenServer {
  public:
    GardenServer(Config& conf) : 
        globals(conf),
        server(80)
    {
    }
    
    void setup();
    
    inline void handleClient() {
      server.handleClient();
    }
    
  private:

    void handleRoot();
    void handleSettings();
    void handleSensor();
    void handleToggleRelay1();
    void handleGenericArgs();
    void handleSpecificArg();
    void handleNotFound();


    String indexProcessor(const String& key);
    String settingsLinksProcessor(const String& key);
    String settingsProcessor(const String& key);
    String settingsPinProcessor(const String& key);
    String settingsPinJsonProcessor(const String& key);
    String settingsGlobalsProcessor(const String& key);
    
    ESP8266WebServer server;
    Config& globals;
    //DhtReader& dht;

};

#endif
