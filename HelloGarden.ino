#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>

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

  pinMode(globals.ledPin, OUTPUT);
  pinMode(5, OUTPUT);

  digitalWrite(globals.ledPin, 0);
  Serial.begin(115200);
  WiFi.begin(globals.ssid.c_str(), globals.password.c_str());
  Serial.println("starting!!!!");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(globals.ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // pages
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

