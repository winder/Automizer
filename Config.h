#ifndef CONFIG_H
#define CONFIG_H

#include "Types.h"
#include "WString.h"
#include "pins_arduino.h"

// #ifdef ESP8266
#define NUM_PINS 8

struct ThirdPartyConfig {
  // Enabled services
  bool useThingSpeak = false;
  bool useBlynk = false;
  bool usePushingBox = false;
  bool useDweet = false;
      
  // Thingspeak globals
  unsigned long thingSpeakChannel =  123456; // YOUR THINGSPEAK CHANNEL
  String thingSpeakKey = "THINGSPEAK-KEY";
  
  // Blynk globals
  String blynkKey = "BLYNK-KEY";
  
  // pushinbox globals
  String pushingBoxKey = "PUSHINGBOX-KEY";
  
  // Dweet globals
  String dweetThing = "DWEET-THING";
};

struct Config {
  // wifi settings
  String ssid = "YOUR-WIFI-SSID";
  String password = "YOUR-WIFI-PASSWORD";

  // Pin config
  int ledPin = LED_BUILTIN;
  int dhtPin = D1;
  
  // DHT Sensor settings
  long minSensorIntervalMs = 2000;
  long uploadInterval = 1000 * 60 * 1;

// Initialize pins based on the ESP8266 mappings...
#ifdef ESP8266
  //const Pin pins[8] = {{D1},{D2},{D3},{D4},{D5},{D6},{D7},{D8}};

  const Pin pins[8] = {
    Pin(D1),
    Pin(D2),
    Pin(D3),
    Pin(D4),
    Pin(D5),
    Pin(D6),
    Pin(D7),
    Pin(D8)
  };
#else
# error "Only ESP8266 boards supported."
#endif

  ThirdPartyConfig thirdPartyConfig;
};

#endif
