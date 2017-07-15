#ifndef CONFIG_H
#define CONFIG_H

#include "Types.h"
#include "pins_arduino.h"

// #ifdef ESP8266
#define NUM_PINS 8
#define KEY_LEN 64
typedef Pin PinArray[NUM_PINS];

struct ThirdPartyConfig {
  // Thingspeak globals
  bool useThingSpeak = false;
  unsigned long thingSpeakChannel = 211407;
  char thingSpeakKey[KEY_LEN] = "CP1YIEMC6F0ZRHVL";
  
  // Blynk globals
  bool useBlynk = false;
  char blynkKey[KEY_LEN] = "d3dc079520c140069eac7ad9221632dd";
  
  // pushinbox globals
  bool usePushingBox = false;
  char pushingBoxKey[KEY_LEN] = "v05F7E2CB04AD208";
  
  // Dweet globals
  bool useDweet = false;
  char dweetThing[KEY_LEN] = "mynkeo_esp8266";

  // For future services
  char reserved[512];
};

struct Config {
  // Pin config
  int ledPin = LED_BUILTIN;

  uint8_t on = HIGH;
  uint8_t off = LOW;
  
  // DHT Sensor settings
  long minSensorIntervalMs = 2000;

  /////////////////////////
  // User configurations //
  /////////////////////////
  bool configInitialized = false;
  
  int32_t timeZoneOffsetMinutes = -5 * 60; // EST (no daylight savings)

  // Interval between updating sensors
  long updateInterval = 60 * 1; // 1 min
  // Interval between updating data
  long uploadInterval = 60 * 15; // 15 min
  

  ////////////////////////////////////
  // Third party api configurations //
  ////////////////////////////////////
  ThirdPartyConfig thirdPartyConfig;
  
  
  ////////////////////////
  // Pin configuration. //
  ////////////////////////

#ifdef ESP8266
  //const Pin pins[NUM_PINS] = {{D1},{D2},{D3},{D4},{D5},{D6},{D7},{D8}};

  // This flag is checked each loop, when false the pins will be re-initialized.
  // The settings page sets this flag to false when they need to be re-initialized.
  bool pinsInitialized = false;

  PinArray pins = {
    Pin(D1),
    Pin(D2),
    Pin(D3, false),
    Pin(D4, false),
    Pin(D5),
    Pin(D6),
    Pin(D7),
    Pin(D8, false)
  };
#else
# error "Only ESP8266 boards supported."
#endif
};

#endif
