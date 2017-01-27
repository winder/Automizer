#ifndef DHT_READER_H
#define DHT_READER_H

#include <DHT.h>

// Data structures
struct dht_data {
  int status;
  boolean failed;
  float temp_f;
  float humidity;
};

class DhtReader {
  public:
      // NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
      // you need to increase the threshold for cycle counts considered a 1 or 0.
      // You can do this by passing a 3rd parameter for this threshold.  It's a bit
      // of fiddling to find the right value, but in general the faster the CPU the
      // higher the value.  The default for a 16mhz AVR is a value of 6.  For an
      // Arduino Due that runs at 84mhz a value of 30 works.
      // This is for the ESP8266 processor on ESP-01 
    DhtReader(int sensorPin, int sensorType, int cycleCounts, unsigned long _minReadIntervalMs) :
        dht(sensorPin, sensorType, cycleCounts),
        minReadIntervalMs(_minReadIntervalMs),
        lastReadMs()
    {
    }
    
    dht_data getTemperature() {  
      unsigned long currentMillis = millis();

      // If the interval has passed update the cached value.
      if(currentMillis - lastReadMs >= minReadIntervalMs) {
        // save the last time you read the sensor 
        lastReadMs = currentMillis;
    
        cache.failed = false;
        
        // Reading temperature for humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
        cache.humidity = dht.readHumidity();          // Read humidity (percent)
        cache.temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
        // Check if any reads failed and exit early (to try again).
        cache.failed = isnan(cache.humidity) || isnan(cache.temp_f);
        if (cache.failed) {
          Serial.println("Failed to read from DHT sensor!");
        }
      }
      
      return cache;
    }

    void dumpTempCache() {
      Serial.print("Failed: "); Serial.println(cache.failed);
      Serial.print("Temperature: "); Serial.println(cache.temp_f);
      Serial.print("Humidity: "); Serial.println(cache.humidity);
      Serial.println("------------------");
    }

    private:
      DHT dht;
      unsigned long minReadIntervalMs;
      unsigned long lastReadMs;
      dht_data cache;
};

#endif
