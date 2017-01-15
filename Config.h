#ifndef CONFIG_H
#define CONFIG_H

struct ThirdPartyConfig {
  // Enabled services
  const bool useThingSpeak = false;
  const bool useBlynk = false;
  const bool usePushingBox = false;
  const bool useDweet = false;
      
  // Thingspeak globals
  const unsigned long thingSpeakChannel =  123456; // YOUR THINGSPEAK CHANNEL
  const String thingSpeakKey = "THINGSPEAK-KEY";
  
  // Blynk globals
  const String blynkKey = "BLYNK-KEY";
  
  // pushinbox globals
  const String pushingBoxKey = "PUSHINGBOX-KEY";
  
  // Dweet globals
  const String dweetThing = "DWEET-THING";
};

struct Config {
  // wifi settings
  const String ssid = "YOUR-WIFI-SSID";
  const String password = "YOUR-WIFI-PASSWORD";

  // Pin config
  const int ledPin = LED_BUILTIN;
  const int dhtPin = D1;
  
  // DHT Sensor settings
  const long minSensorIntervalMs = 2000;
  const long uploadInterval = 1000 * 60 * 1;

  ThirdPartyConfig thirdPartyConfig;
};

#endif
