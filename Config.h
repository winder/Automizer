#ifndef CONFIG_H
#define CONFIG_H

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

  ThirdPartyConfig thirdPartyConfig;
};

#endif
