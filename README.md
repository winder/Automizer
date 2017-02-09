# README #

Control a bunch of outlets from an ESP8266.

### Setup ###

Using the Arduino IDE and [esp8266/Arduino](https://github.com/esp8266/Arduino) SDK.

Libraries:
* [WifiManager](https://github.com/tzapu/WiFiManager)
* [NTPClient](https://github.com/arduino-libraries/NTPClient)
* [Arduino JSON](https://github.com/bblanchon/ArduinoJson)

Data upload:
Files are served from SPIFFS storage, these files are stored in the data directory. To upload the files to your esp8266 you can use the [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin).

### Usage ###

When first installing, the device will load into access point mode. Connect to "ThingBot" and configure your wifi.

Once wifi is configured the device will boot up. I'm not sure how you're supposed to find the IP address without looking on your router.