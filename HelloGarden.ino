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
  // Using a custom initialization until I get the json config setter/loader figured out.
  //return false;

  globals.pins[0].type = PinType_Input_TempSensorDHT11;
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
  unsigned long timestamp = timeClient.getEpochTime();
  updateSettings();
  gardenServer.handleClient();
  //Serial.println(timeClient.getFormattedTime());
  updateSensors(timestamp);
  enabler.update(timestamp);
  uploadData(timestamp);
}

// Check if the settings have been changed:
// globals.configInitialized - dirty flag for general configuration
// globals.pinsInitialized   - dirty flag for pin configuration
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

// TODO: Use a timer utility.
unsigned long previousUploadSec = 0;
void uploadData(unsigned long currentSec) {
  // Sometimes things get funny when starting
  if (previousUploadSec > currentSec) previousUploadSec = currentSec;
  
  if(currentSec - previousUploadSec >= globals.uploadInterval) {
    Serial.println(String("========= upload ========="));
    Serial.println(String("curr:     ") + currentSec + "\nprevious: " + previousUploadSec + "\ninterval: " + globals.uploadInterval
    +"\ndifference: " + (currentSec - previousUploadSec));
    
    //Serial.println(String("Upload Data Interval: ") + globals.uploadInterval + ", " + currentMillis);    
    
    bool failed = true;
    for (int i=0; i < dhtReaders.size(); i++) {
      std::pair<DhtReader,int>& dht = dhtReaders[i];
      // Only stage good data.
      if (!globals.pins[dht.second].tempData.failed) {
        failed = false;
        String name = String(globals.pins[dht.second].name);
        integrations.stage(String("temp_") + name, globals.pins[dht.second].tempData.temp_f);
        integrations.stage(String("humidity_") + name, globals.pins[dht.second].tempData.humidity);   
      }
    }
    integrations.uploadStagedData();

    if (!failed) {
      previousUploadSec = currentSec;
    } else {
      previousUploadSec += 1;
    }
  }
}

// TODO: Use a timer utility.
unsigned long previousUpdateSec = 0;
bool updateSensors(unsigned long currentSec) {
  // Sometimes things get funny when starting
  if (previousUpdateSec > currentSec) previousUpdateSec = currentSec;
  
  if(currentSec - previousUpdateSec >= globals.updateInterval) {
    Serial.println(String("========= update ========="));
    Serial.println(String("curr:     ") + currentSec + "\nprevious: " + previousUpdateSec + "\ninterval: " + globals.updateInterval
    +"\ndifference: " + (currentSec - previousUpdateSec));
    //Serial.println(String("Update Sensor Interval: ") + globals.updateInterval + ", " + currentMillis);

    // Save sensor data to Pin struct
    bool failed = false;
    for (int i=0; i < dhtReaders.size(); i++) {
      std::pair<DhtReader,int>& dht = dhtReaders[i];
      globals.pins[dht.second].tempData = dht.first.getTemperature();
      if (globals.pins[dht.second].tempData.failed) {
        Serial.println(String("Failed to get temperature data for pin: ") + dht.second);
        failed = true;
      } else {
        Serial.print("Pin: "); Serial.println(dht.second);
        dht.first.dumpTempCache();
      }
    }
    
    if (!failed) {
      // integrations.uploadStaged();
      previousUpdateSec = currentSec;
    } else {
      // Don't try more than once a second.
      previousUpdateSec += 1;
    }
    
    return failed;
  }
  
  return true;
}

