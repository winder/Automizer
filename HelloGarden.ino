#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManager.h>

#include "DhtReader.h"
#include "ThirdPartyIntegrations.h"
#include "Config.h"
#include "ConfigSetter.h"
#include "GardenServer.h"
#include "DhtReader.h"

// Globals
Config globals;
DhtReader dht(globals.dhtPin, DHT11, 16, globals.minSensorIntervalMs);
GardenServer gardenServer(globals, dht);

// Helper objects
ThirdPartyIntegrations integrations(globals.thirdPartyConfig);

// Setup server.
void setup(void){
  SPIFFS.begin();
  loadConfig(globals);

  pinMode(globals.ledPin, OUTPUT);
  pinMode(5, OUTPUT);

  digitalWrite(globals.ledPin, 0);
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("ThingBot");

  // HTTP Server
  gardenServer.setup();
  Serial.println("HTTP server started");

  digitalWrite(globals.ledPin, LOW);
}

// Main loop.
// Check for client connections and upload data on an interval.
void loop(void){
  gardenServer.handleClient();
  uploadData();
}

unsigned long previousUploadMillis = 0;
void uploadData() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousUploadMillis >= globals.uploadInterval) {

    dht_data data = dht.getTemperature();
    if (!data.failed) {
      Serial.println("Good data received, uploading.");
      dht.dumpTempCache();
      
      // Got a good reading, reset the previous upload time.
      previousUploadMillis = currentMillis;
  
      String t = String(data.temp_f, 2);
      String h = String(data.humidity, 2);

      integrations.upload(t, h);
    }
  }
}

