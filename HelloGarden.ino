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

Serial.println("initializing...");
  globals.pins[0].type = PinType_Input_TempSensorDHT11;
  globals.pins[1].type = PinType_Input_TempSensorDHT11;
  
  globals.pins[4].type = PinType_Output_Relay;
  globals.pins[4].data.outputConfig.trigger = OutputTrigger_Temperature;
  globals.pins[4].data.outputConfig.tempConfig.sensorIndex = 1;
  globals.pins[4].data.outputConfig.tempConfig.temperatureTrigger = SensorTriggerType_Above;
  globals.pins[4].data.outputConfig.tempConfig.temperatureThreshold = 80;
  globals.pins[4].data.outputConfig.tempConfig.humidityTrigger = SensorTriggerType_Disabled;
  globals.pins[4].data.outputConfig.tempConfig.humidityThreshold = 50;
  
  globals.pins[5].type = PinType_Output_Relay;
  globals.pins[5].data.outputConfig.trigger = OutputTrigger_Temperature;
  globals.pins[5].data.outputConfig.tempConfig.sensorIndex = 1;
  globals.pins[5].data.outputConfig.tempConfig.temperatureTrigger = SensorTriggerType_Below;
  globals.pins[5].data.outputConfig.tempConfig.temperatureThreshold = 80;
  globals.pins[5].data.outputConfig.tempConfig.humidityTrigger = SensorTriggerType_Disabled;
  globals.pins[5].data.outputConfig.tempConfig.humidityThreshold = 50;

  globals.pins[6].type = PinType_Output_Relay;
  globals.pins[6].data.outputConfig.trigger = OutputTrigger_Schedule;
  globals.pins[6].data.outputConfig.scheduleConfig.startMinutes = 61;
  globals.pins[6].data.outputConfig.scheduleConfig.stopMinutes = 21;

  dumpPin(globals.pins[0], 0);
  dumpPin(globals.pins[4], 4);
  dumpPin(globals.pins[5], 5);
  dumpPin(globals.pins[6], 6);
}

// Setup server.
void setup(void){
  Serial.begin(115200);

  SPIFFS.begin();
  if (!customInitialization(globals)) {
    //loadConfig(globals);
  }
  
  globals.pinsInitialized = false;
  globals.configInitialized = false;  

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
  enabler.update(timeClient);
  updateSensors(timestamp);
  //uploadData(timestamp);
  return;
}

// Check if the settings have been changed:
// globals.configInitialized - dirty flag for general configuration
// globals.pinsInitialized   - dirty flag for pin configuration
void updateSettings() {
  if (!globals.configInitialized) {
    timeClient.setTimeOffset(globals.timZoneOffsetMinutes * 60);
    globals.configInitialized = true;
  }
  
  if (!globals.pinsInitialized) {
    dhtReaders.clear();
    
    // Initialize the pins
    int numPins = NUM_PINS;
    for (int i = 0; i < numPins; i++) {
      switch (globals.pins[i].type) {
        case PinType_Input_TempSensorDHT11:
          Serial.println(String("Creating DHT11 on pin: ") + (i+1));
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(std::make_pair(DhtReader(globals.pins[i].pinNumber, DHT11, 16, globals.minSensorIntervalMs), i));
          break;
        case PinType_Input_TempSensorDHT22:
          Serial.println(String("Creating DHT11 on pin: ") + (i+1));
          pinMode(globals.pins[i].pinNumber, INPUT);
          dhtReaders.push_back(std::make_pair(DhtReader(globals.pins[i].pinNumber, DHT22, 16, globals.minSensorIntervalMs), i));
          break;
        case PinType_Output_Relay:
          Serial.println(String("Creating RELAY on pin: ") + (i+1));
          pinMode(globals.pins[i].pinNumber, OUTPUT);
          digitalWrite(globals.pins[i].pinNumber, OFF);
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
    //Serial.println(String("========= upload ========="));
    //Serial.println(String("curr:     ") + currentSec + "\nprevious: " + previousUploadSec + "\ninterval: " + globals.uploadInterval +"\ndifference: " + (currentSec - previousUploadSec));
    
    bool failed = true;
    for (int i=0; i < dhtReaders.size(); i++) {
      std::pair<DhtReader,int>& dht = dhtReaders[i];
      // Only stage good data.
      if (!globals.pins[dht.second].data.tempData.failed) {
        failed = false;
        String name = String(globals.pins[dht.second].name);
        integrations.stage(String("temp_") + name, globals.pins[dht.second].data.tempData.temp_f);
        integrations.stage(String("humidity_") + name, globals.pins[dht.second].data.tempData.humidity);   
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
    //Serial.println(String("========= update ========="));
    //Serial.println(String("curr:     ") + currentSec + "\nprevious: " + previousUpdateSec + "\ninterval: " + globals.updateInterval +"\ndifference: " + (currentSec - previousUpdateSec));

    // Save sensor data to Pin struct
    bool failed = false;
    for (int i=0; i < dhtReaders.size(); i++) {
      std::pair<DhtReader,int>& dht = dhtReaders[i];
      globals.pins[dht.second].data.tempData = dht.first.getTemperature();
      if (globals.pins[dht.second].data.tempData.failed) {
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

