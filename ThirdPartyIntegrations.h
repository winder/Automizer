#ifndef THIRD_PARTY_INTEGRATIONS_H
#define THIRD_PARTY_INTEGRATIONS_H

#include "ThingSpeak.h"
#include "Config.h"
#include <BlynkSimpleEsp8266.h>

class ThirdPartyIntegrations {
  public:
    ThirdPartyIntegrations(ThirdPartyConfig config) :
    /*
      bool _useThingSpeak, unsigned long _thingSpeakChannel, String _thingSpeakKey,
      bool _useBlynk, String _blynkKey,
      bool _usePushingBox, String _pushnBoxKey,
      bool _useDweet, String _dweetThing) :*/
        useThingSpeak(config.useThingSpeak),
        thingSpeakFailedCounter(0),
        thingSpeakChannel(config.thingSpeakChannel),
        thingSpeakKey(config.thingSpeakKey),
        useBlynk(config.useBlynk),
        blynkKey(config.blynkKey),
        usePushingBox(config.usePushingBox),
        pushingBoxKey(config.pushingBoxKey),
        useDweet(config.useDweet),
        dweetThing(config.dweetThing)
    {
    }

    void upload(String temperature, String humidity) {
      Serial.println("UPLOAD DATA");

      if (useThingSpeak)  sendThingspeak(temperature, humidity);
      if (useBlynk)       sendBlynk(temperature, humidity);
      if (usePushingBox)  sendPushingBox(temperature, humidity);
      if (useDweet)       sendDweet(temperature, humidity);
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
    client.print(String("GET /dweet/for/") + dweetThing + "?" + data +
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
    client.print("X-THINGSPEAKAPIKEY: "+thingSpeakKey+"\n");
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
      Blynk.config(blynkKey.c_str());
      Serial.println("Not connected to Blynk server, reconnecting"); 
      Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
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
  
    String url = "GET /pushingbox?devid=" + pushingBoxKey +
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
    
  private:
    int thingSpeakFailedCounter;

    const bool useThingSpeak;
    const bool useBlynk;
    const bool usePushingBox;
    const bool useDweet;
    
    const long thingSpeakChannel;
    const String thingSpeakKey;
    const String blynkKey;
    const String pushingBoxKey;
    const String dweetThing;
};

#endif
