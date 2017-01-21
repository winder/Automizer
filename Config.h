#ifndef CONFIG_H
#define CONFIG_H

#include "Types.h"
#include "pins_arduino.h"

// #ifdef ESP8266
#define NUM_PINS 8
#define KEY_LEN 64

struct ThirdPartyConfig {
  // Thingspeak globals
  bool useThingSpeak = false;
  unsigned long thingSpeakChannel =  123456; // YOUR THINGSPEAK CHANNEL
  char thingSpeakKey[KEY_LEN] = "THINGSPEAK-KEY";
  
  // Blynk globals
  bool useBlynk = false;
  char blynkKey[KEY_LEN] = "BLYNK-KEY";
  
  // pushinbox globals
  bool usePushingBox = false;
  char pushingBoxKey[KEY_LEN] = "PUSHINGBOX-KEY";
  
  // Dweet globals
  bool useDweet = false;
  char dweetThing[KEY_LEN] = "DWEET-THING";

  // For future services
  char reserved[512];
};

struct Config {
  // Pin config
  int ledPin = LED_BUILTIN;
  int dhtPin = D1;
  
  // DHT Sensor settings
  long minSensorIntervalMs = 2000;
  long uploadInterval = 1000 * 60 * 1;

  char reserved[256];
  
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
