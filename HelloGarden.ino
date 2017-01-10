#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "ThingSpeak.h"
#include <BlynkSimpleEsp8266.h>

// Data structures
struct dht_data {
  int status;
  boolean failed;
  float temp_f;
  float humidity;
};

// Upload globals
#define UPLOAD_INTERVAL_MINUTES 1
unsigned long previousUploadMillis = 0;
const long uploadInterval = 1000 * 60 * UPLOAD_INTERVAL_MINUTES;

// Thingspeak globals
int failedCounter = 0;
const unsigned long myChannelNumber =  123456; // YOUR THINGSPEAK CHANNEL
const String writeAPIKey = "THINGSPEAK-KEY";

// Blynk globals
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char blynkKey[] = "BLYNK-KEY";
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

// pushinbox globals
String pushinBoxDevid = "PUSHINGBOX-KEY";

// wifi globals
const char* ssid = "YOUR-WIFI-SSID";
const char* password = "YOUR-WIFI-PASSWORD";
ESP8266WebServer server(80);

// DHT globals
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor
dht_data cache;
#define DHTTYPE DHT11
#define DHTPIN  D1
#define LEDPIN LED_BUILTIN
// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 16); // 11 works fine for ESP8266
// Generally, you should use "unsigned long" for variables that hold time


void handleRoot() {
  digitalWrite(LEDPIN, 1);
  server.send(200, "text/plain", "hello from esp8266!!!!");
  digitalWrite(LEDPIN, 0);
}

void handleSensor() {
  dht_data data = getTemperature();       // read sensor

  String webString="Temperature: "+String(data.temp_f, 2)+" F";   // Arduino has a hard time with float to string
  webString += "\nHumidity: "+String(data.humidity, 2)+"%";

  if (data.failed) {
    webString = "Failed to read sensor data.";
  }

  server.send(200, "text/plain", webString);            // send to someones browser when asked
}

void handleNotFound(){
  digitalWrite(LEDPIN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LEDPIN, 0);
}

// Setup server.
void setup(void){
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("starting!!!!");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // pages
  server.on("/", handleRoot);
  server.on("/dht", handleSensor);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  digitalWrite(LEDPIN, LOW);
}

// Main loop.
// Check for client connections and upload data on an interval.
void loop(void){
  server.handleClient();
  uploadData();
}

void dumpTempCache() {
  Serial.print("Failed: "); Serial.println(cache.failed);
  Serial.print("Temperature: "); Serial.println(cache.temp_f);
  Serial.print("Humidity: "); Serial.println(cache.humidity);
  Serial.println("------------------");
}

dht_data getTemperature() {  
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    Serial.println("Reading DHT sensor.");


    // save the last time you read the sensor 
    previousMillis = currentMillis;

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

// Send data via dweet.io
void sendDweet(String t, String h) {
  String data = "temperature=" + h + "&humidity=" + t;
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
  client.print(String("GET /dweet/for/DWEET-THING?") + data +
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
    failedCounter++;
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
    Serial.println();
    return;
  }
    
  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(tsData.length());
  client.print("\n\n");
  client.print(tsData);
  if (client.connected()) {
    Serial.println("Connecting to ThingSpeak...");
    Serial.println();
    failedCounter = 0;
  }
  else {
    failedCounter++;
    Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
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
void sendBylnk(String t, String h) {
  if(!Blynk.connected()){
    Blynk.config(blynkKey);
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

  String url = "GET /pushingbox?devid=" + pushinBoxDevid +
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

void uploadData() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousUploadMillis >= uploadInterval) {

    dht_data data = getTemperature();
    if (!data.failed) {
      Serial.println("Good data received, uploading.");
      dumpTempCache();
      
      // Got a good reading, reset the previous upload time.
      previousUploadMillis = currentMillis;
  
      String t = String(data.temp_f, 2);
      String h = String(data.humidity, 2);
  
      // Upload all the data.
      
      sendDweet(t, h);
      
      sendThingspeak(t, h);
   
      sendBylnk(t, h);
  
      sendPushingBox(t, h);
    }
  }
}

