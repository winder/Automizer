#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "DhtReader.h"
#include "ThirdPartyIntegrations.h"
#include "Config.h"

// Pin globals
#define RELAY_1 D5

// Globals
Config globals;

// Helper objects
DhtReader dht(globals.dhtPin, DHT11, 16, globals.minSensorIntervalMs);
ThirdPartyIntegrations integrations(globals.thirdPartyConfig);
ESP8266WebServer server(80);

  
void handleRoot() {
  digitalWrite(globals.ledPin, 1);
  server.send(200, "text/plain", "hello from esp8266!!!!");
  digitalWrite(globals.ledPin, 0);
}

bool relay1 = 0;
void handleToggleRelay1() {
  relay1 = !relay1;
  server.send(200, "text/plain", String("Relay 1 state: ") + relay1);
  digitalWrite(RELAY_1, relay1);
}

void handleGenericArgs() { //Handler
  String message = "Number of args received:";
  message += server.args();            //Get number of parameters
  message += "\n";                            //Add a new line
  
  for (int i = 0; i < server.args(); i++) {
    message += "Arg nº" + (String)i + " –> ";   //Include the current iteration value
    message += server.argName(i) + ": ";     //Get the name of the parameter
    message += server.arg(i) + "\n";              //Get the value of the parameter
  } 
  
  server.send(200, "text/plain", message);       //Response to the HTTP request
}

void handleSpecificArg() { 
  String message = "";
  
  if (server.arg("Temperature")== ""){     //Parameter not found
    message = "Temperature Argument not found";
  } else {     //Parameter found
    message = "Temperature Argument = ";
    message += server.arg("Temperature");     //Gets the value of the query parameter
  }

  server.send(200, "text/plain", message);
}

void handleSensor() {
  // read sensor
  dht_data data = dht.getTemperature();

  // Arduino has a hard time with float to string
  String webString="Temperature: "+String(data.temp_f, 2)+" F";
  webString += "\nHumidity: "+String(data.humidity, 2)+"%";

  if (data.failed) {
    webString = "Failed to read sensor data.";
  }

  // send to someones browser when asked
  server.send(200, "text/plain", webString);
}

void handleNotFound(){
  digitalWrite(globals.ledPin, 1);
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
  digitalWrite(globals.ledPin, 0);
}

// Setup server.
void setup(void){
  pinMode(globals.ledPin, OUTPUT);
  pinMode(RELAY_1, OUTPUT);

  digitalWrite(globals.ledPin, 0);
  Serial.begin(115200);
  WiFi.begin(globals.ssid.c_str(), globals.password.c_str());
  Serial.println("starting!!!!");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(globals.ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // pages
  server.on("/", handleRoot);
  server.on("/dht", handleSensor);
  server.on("/relay1", handleToggleRelay1);
  server.on("/argTest1", handleGenericArgs);
  server.on("/argTest2", handleSpecificArg);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  digitalWrite(globals.ledPin, LOW);
}

// Main loop.
// Check for client connections and upload data on an interval.
void loop(void){
  server.handleClient();
  uploadData();
}

unsigned long previousUploadMillis = 0;
void uploadData() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousUploadMillis >= globals.uploadInterval) {

    dht_data data = dht.getTemperature();
    if (!data.failed) {
      Serial.println("Good data received, uploading.");
      dht.dumpTempCache();
      
      // Got a good reading, reset the previous upload time.
      previousUploadMillis = currentMillis;
  
      String t = String(data.temp_f, 2);
      String h = String(data.humidity, 2);

      integrations.upload(t, h);
    }
  }
}

