#ifndef THIRD_PARTY_INTEGRATIONS_H
#define THIRD_PARTY_INTEGRATIONS_H

#include <BlynkSimpleEsp8266.h>
#include <utility>
#include <vector>

#include "ThingSpeak.h"
#include "Config.h"

class ThirdPartyIntegrations {
      
  private:
    int thingSpeakFailedCounter;

    // Key-Value pair that needs to be sent
    std::vector<std::pair<String, String>> stagedData;

    // Reference to global settings.
    ThirdPartyConfig& conf;
    
  public:
    ThirdPartyIntegrations(ThirdPartyConfig& config) :
        conf(config),
        thingSpeakFailedCounter(0)
    {}

    void upload(float &temperature, float &humidity) {
      upload(String(temperature), String(humidity));
    }

    void upload(String temperature, String humidity) {
      Serial.println("UPLOAD DATA");

      if (conf.useThingSpeak)  sendThingspeak(temperature, humidity);
      if (conf.useBlynk)       sendBlynk(temperature, humidity);
      if (conf.usePushingBox)  sendPushingBox(temperature, humidity);
      if (conf.useDweet)       sendDweet(temperature, humidity);
    }

    void uploadStagedData() {
      Serial.println(String("Staged data size: ") + stagedData.size());
      //upload
      resetStagedData();
    }

    void resetStagedData() {
      stagedData.clear();
    }
    
    void stage(String& key, String value) {
      // sanitize the key
      String k = String(key);
      for (int i = 0; i < k.length(); i++) {
        if (k[i] == ' ') k[i] = '_';
        if (isalnum(k[i]) == 0) {
          k.remove(i);
          i--;
        }
      }
      stagedData.push_back(std::make_pair(key, value));
    }

    void stage(String& key, int value) {
      stage(key, String(value, DEC));
    }

    void stage(String& key, float value) {
      stage(key, String(value, DEC));
    }

  private:
  
  // Send data via dweet.io
  void sendDweet(String t, String h) {
    String data = "temperature=" + t + "&humidity=" + h;
    sendDweet(data);
  }
  
  void sendDweet(String data) {
    String host = "dweet.io";
    Serial.println("Connecting to " + host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(host.c_str(), 80)) {
      Serial.println("connection failed");
      return;
    }
  
    // This will send the request to the server
    client.print(String("GET /dweet/for/") + conf.dweetThing + "?" + data +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
    delay(10);
  
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    Serial.println();
    Serial.println("closing connection");
    client.stop();
  }
  
  // Send data via thingspeak
  void sendThingspeak(String t, String h) {
    String params = "field1="+t+"&field2="+h;
    sendThingspeak(params);
  }
  
  // Send generic parameter data to thingspeak.
  void sendThingspeak(String tsData) {
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect("api.thingspeak.com", 80)) {
      thingSpeakFailedCounter++;
      Serial.println("Connection to ThingSpeak Failed ("+String(thingSpeakFailedCounter, DEC)+")");
      Serial.println();
      return;
    }
      
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: ");
    client.print(conf.thingSpeakKey);
    client.print("\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    if (client.connected()) {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      thingSpeakFailedCounter = 0;
    }
    else {
      thingSpeakFailedCounter++;
      Serial.println("Connection to ThingSpeak failed ("+String(thingSpeakFailedCounter, DEC)+")");
      Serial.println();
    }
  
    delay(10);
  
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
  }
  
  // Send data via blynk.cc
  void sendBlynk(String t, String h) {
    if(!Blynk.connected()){
      Blynk.config(conf.blynkKey);
      Serial.println("Not connected to Blynk server, reconnecting"); 
      Blynk.connect(10);  // timeout set to 10 seconds and then continue without Blynk
      while (Blynk.connect() == false) {
        // Wait until connected
      }
    }
  
    if (!Blynk.connected()) {
      Serial.println("Failed to reconnect to blynk, aborting.");
      return;
    }
  
    Serial.println("Writing to blynk...");
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
  
    Serial.println("Done with blynk...");
    Blynk.disconnect();
  }
  
  void sendPushingBox(String t, String h) {
    String host = "api.pushingbox.com";
    Serial.println("Connecting to " + host);
  
    String url = "GET /pushingbox?devid=" + String(conf.pushingBoxKey) +
        "&temp=" + t +
        "&humidity=" + h;
        
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(host.c_str(), 80)) {
      Serial.println("connection failed");
      return;
    }
  
    // This will send the request to the server
    client.print(url +
               " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
    client.stop();
  }
};

#endif
