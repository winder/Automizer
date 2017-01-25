#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManager.h>
#include <vector>

#include "DhtReader.h"
#include "ThirdPartyIntegrations.h"
#include "Config.h"
#include "ConfigSetter.h"
#include "GardenServer.h"
#include "DhtReader.h"

// Globals
Config globals;
std::vector<DhtReader> dhtReaders;
GardenServer gardenServer(globals);

// Helper objects
ThirdPartyIntegrations integrations(globals.thirdPartyConfig);

bool customInitialization(Config& config) {
  return false;

  globals.pins[0].type = Input_TempSensorDHT11;
  globals.pins[1].type = Output_Relay;
}
// Setup server.
void setup(void){
  SPIFFS.begin();
  if (!customInitialization(globals)) {
    loadConfig(globals);
  }
  
  globals.pinsInitialized = false;
  
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
}

// Main loop.
// Check for client connections and upload data on an interval.
void loop(void){
  initializePins();
  gardenServer.handleClient();
  uploadData();
}


void initializePins() {
  if (!globals.pinsInitialized) {
    //pinMode(globals.ledPin, OUTPUT);

    dhtReaders.clear();
    
    // Initialize the pins
    //int numPins = sizeof(globals.pins)/sizeof(*(globals.pins));
    int numPins = NUM_PINS;
    for (int i = 0; i < numPins; i++) {
      switch (globals.pins[i].type) {
        case Input_TempSensorDHT11:
          Serial.print("CREATING DHT11 ON PIN: "); Serial.println(i+1);
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(DhtReader(globals.pins[i], DHT11, 16, globals.minSensorIntervalMs));
          break;
        case Input_TempSensorDHT22:
          Serial.println("CREATING DHT22");
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(DhtReader(globals.pins[i], DHT22, 16, globals.minSensorIntervalMs));
          break;
        case Output_Relay:
          Serial.println("CREATING RELAY");
          pinMode(globals.pins[i].pinNumber, OUTPUT);
          // TODO: Configure relay triggers
          break;
        default:
          break;
      }
    }
    
    globals.pinsInitialized = true;
  }
}

  
unsigned long previousUploadMillis = 0;
void uploadData() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousUploadMillis >= globals.uploadInterval) {
    Serial.println(String("Interval: ") + globals.uploadInterval);

    std::vector<dht_data> dataVector;
    bool failed = false;
    for (int i=0; !failed && i < dhtReaders.size(); i++) {
      DhtReader& dht = dhtReaders[i];
      dht_data data = dht.getTemperature();
      if (!data.failed) {
        dataVector.push_back(data);

        Serial.println("Good data received, uploading.");
        dht.dumpTempCache();
    
        //String t = String(data.temp_f, 2);
        //String h = String(data.humidity, 2);
    
        //integrations.upload(t, h);
      } else {
        failed = true;
      }
    }

    // If we got a good reading, reset the previous upload time.
    if (!failed) {
      previousUploadMillis = currentMillis;
    } else {
      // Don't try more than once a second.
      previousUploadMillis += 1000;
    }
  }
}

