#ifndef ENABLER_H
#define ENABLER_H

#include <cstring>
#include "Config.h"
#include "Types.h"

class Enabler {
  private:
    struct pin_meta {
      // Marked if the pin configuration should be recalculated.
      bool stale = true;
      
      // Depending on the Pin type this can be a:
      // temperature, humidity, or timestamp.
      unsigned long changeAt;
      // For hysteresis save the timestamp of the last toggle.
      unsigned long lastChange;
      // The current enable state
      bool enabled;
    };
    
  public:
    Enabler(PinArray& _pins, int _numPins) : pins(_pins), numPins(_numPins) {}

    void update(unsigned long currentTimestamp) {
      for (int i = 0; i < numPins; i++) {
        if (state[i].stale) {
          // set timestamp for next toggle
          state[i].stale = false;

          // Reset data/configuration
          std::memset(&(pins[i].data), 0, sizeof(pins[i].data));
        }
        switch (pins[i].type) {
          case PinType_Input_TempSensorDHT11:
          case PinType_Input_TempSensorDHT22:
            //Serial.println("ENABLER");
            //Serial.println(String("F: ") + pins[i].tempData.temp_f);
            //Serial.println(String("H: ") + pins[i].tempData.humidity);            
            break;
        }
      }
    }

  private:

    void updateState(Pin& p, pin_meta& meta) {
      
    }

    // Metadata used to determine if the pin needs to toggle.
    pin_meta state[NUM_PINS]; 

    // Reference to the pins.
    const int numPins;
    PinArray& pins;
};

#endif
