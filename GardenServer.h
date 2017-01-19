#ifndef GARDEN_SERVER_H
#define GARDEN_SERVER_H

#include "Config.h"
#include "DhtReader.h"
#include <ESP8266WebServer.h>

class GardenServer {
  public:
    GardenServer(Config& conf, DhtReader& reader) : 
        globals(conf),
        server(80),
        dht(reader)
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

    
    ESP8266WebServer server;
    Config& globals;
    DhtReader& dht;

};

#endif
