#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManager.h>
#include <vector>
#include <WiFiUDP.h>
#include <NTPClient.h>

#include "DhtReader.h"
#include "ThirdPartyIntegrations.h"
#include "Config.h"
#include "ConfigSetter.h"
#include "GardenServer.h"
#include "DhtReader.h"
#include "Enabler.h"

// Globals
Config globals;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Map the DhtReader to the pin array index.
std::vector<std::pair<DhtReader,int>> dhtReaders;
GardenServer gardenServer(globals);
Enabler enabler(globals.pins, NUM_PINS);

// Helper objects
ThirdPartyIntegrations integrations(globals.thirdPartyConfig);

bool customInitialization(Config& config) {
  //return false;

  globals.pins[1].type = PinType_Input_TempSensorDHT11;
  globals.pins[2].type = PinType_Output_Relay;
}

// Setup server.
void setup(void){
  SPIFFS.begin();
  if (!customInitialization(globals)) {
    //loadConfig(globals);
  }
  
  globals.pinsInitialized = false;
  globals.configInitialized = false;
  
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
  timeClient.begin();
}

// Main loop.
// Check for client connections and upload data on an interval.
void loop(void){
  timeClient.update();
  updateSettings();
  gardenServer.handleClient();
  //Serial.println(timeClient.getFormattedTime());
  uploadData();
}


void updateSettings() {
  if (!globals.configInitialized) {
    timeClient.setTimeOffset(globals.timZoneOffsetMinutes);
    globals.configInitialized = true;
  }
  if (!globals.pinsInitialized) {
    dhtReaders.clear();
    
    // Initialize the pins
    int numPins = NUM_PINS;
    for (int i = 0; i < numPins; i++) {
      switch (globals.pins[i].type) {
        case PinType_Input_TempSensorDHT11:
          Serial.print("CREATING DHT11 ON PIN: "); Serial.println(i+1);
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(std::make_pair(DhtReader(globals.pins[i].pinNumber, DHT11, 16, globals.minSensorIntervalMs), i));
          break;
        case PinType_Input_TempSensorDHT22:
          Serial.println("CREATING DHT22");
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(std::make_pair(DhtReader(globals.pins[i].pinNumber, DHT22, 16, globals.minSensorIntervalMs), i));
          break;
        case PinType_Output_Relay:
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

    bool failed = false;
    for (int i=0; !failed && i < dhtReaders.size(); i++) {
      std::pair<DhtReader,int>& dht = dhtReaders[i];
      globals.pins[dht.second].tempData = dht.first.getTemperature();
      if (!globals.pins[dht.second].tempData.failed) {
        Serial.println("Good data received, uploading.");
        Serial.println(String("F: ") + globals.pins[dht.second].tempData.temp_f);
        Serial.println(String("H: ") + globals.pins[dht.second].tempData.humidity);
        dht.first.dumpTempCache();
    
        //String t = String(data.temp_f, 2);
        //String h = String(data.humidity, 2);
    
        //integrations.upload(t, h);
      } else {
        Serial.println("Failed to get temperature data.");
        failed = true;
      }
    }

    enabler.update(1234);
    // If we got a good reading, reset the previous upload time.
    if (!failed) {
      previousUploadMillis = currentMillis;
    } else {
      // Don't try more than once a second.
      previousUploadMillis += 1000;
    }
  }
}

