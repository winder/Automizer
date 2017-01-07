#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "YOUR-WIFI-SSID";
const char* password = "YOUR-WIFI-PASSWORD";

ESP8266WebServer server(80);

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

struct dht_data {
  float temp_f;
  float humidity;
};

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor


void handleRoot() {
  digitalWrite(LEDPIN, 1);
  Serial.println("gettemperature()");

  server.send(200, "text/plain", "hello from esp8266!!!!");
  digitalWrite(LEDPIN, 0);
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
  digitalWrite(LEDPIN, LOW);
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/dht", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    dht_data data = gettemperature();       // read sensor
    
    String webString="Temperature: "+String(data.temp_f, 2)+" F";   // Arduino has a hard time with float to string
    webString += "\nHumidity: "+String(data.humidity, 2)+"%";

    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  
}

dht_data cache;
dht_data gettemperature() {  
  Serial.println("gettemperature()");

  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    cache.humidity = dht.readHumidity();          // Read humidity (percent)
    cache.temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
    Serial.println(String((int)cache.temp_f) + " F, " + String((int)cache.humidity) + " % humidity");
    // Check if any reads failed and exit early (to try again).
    if (isnan(cache.humidity) || isnan(cache.temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
    }
  }
  return cache;
}
