#include "GardenServer.h"

#include "ConfigSetter.h"
#include "DhtReader.h"
#include "ESPTemplateProcessor.h"
#include <vector>

#define BIND(NAME) std::bind(&GardenServer::NAME, this)
#define BIND_PROCESSOR(NAME) std::bind(&GardenServer::NAME, this, std::placeholders::_1);

void GardenServer::setup() {
      // pages
      server.on("/", BIND(handleRoot));
      server.on("/settings", BIND(handleSettings));
      server.on("/integrationSettings", BIND(handleSettings));
      server.on("/pinSettings", BIND(handleSettings));
      server.on("/pinSettingsJson", BIND(handleSettings));
      server.on("/dht", BIND(handleSensor));
      server.on("/relay1", BIND(handleToggleRelay1));
      server.on("/argTest1", BIND(handleGenericArgs));
      server.on("/argTest2", BIND(handleSpecificArg));
      server.onNotFound(BIND(handleNotFound));
    
      server.begin();
}

String GardenServer::indexProcessor(const String& key) {
  return "Hello Garden";
}

String GardenServer::settingsProcessor(const String& key) {
  if (key == "TITLE") return "Gardenbot Settings";
  else if (key == "BODY") return getIntegrationSettingsBody(globals);
}

String GardenServer::settingsPinProcessor(const String& key) {
  if (key == "TITLE") return "Gardenbot Pin Settings";
  else if (key == "BODY") return getPinSettingsBody(globals);
}

String GardenServer::settingsPinJsonProcessor(const String& key) {
  if (key == "TITLE") return "Gardenbot Pin Settings";
  else if (key == "NUM_PINS") return String(NUM_PINS, DEC);
  else if (key == "DEFAULTS") {
    int maxSize = sizeof(Config) * 2;
    std::vector<char> buffer;
    buffer.reserve(maxSize);
    if(configToJson(globals, &buffer[0], maxSize)) {
      return String(&buffer[0]);
    }
  }
}

String GardenServer::settingsLinksProcessor(const String& key) {
  if (key == "TITLE") return "Gardenbot Settings";
  else if (key == "BODY") return getSettingsLinksBody();
}

void GardenServer::handleRoot() {
  ProcessorCallback cb = BIND_PROCESSOR(indexProcessor);

  if (ESPTemplateProcessor(server).send(String("/index.html"), cb)) {
    Serial.println("SUCCESS");
  } else {
    Serial.println("FAIL");
    server.send(200, "text/plain", "FUBAR!!!");
  }
}

void GardenServer::handleSettings() {
  Serial.print("handleSettings: "); Serial.println(server.uri());
  
  // Process submitted settings
  if (server.uri() == "/submitIntegrationSettings") {
    processIntegrationResults(server, globals);
    saveConfig(globals);
    globals.pinsInitialized = false;
  }
  else if (server.uri() == "/submitPinSettings") {
    processPinResults(server, globals);
    saveConfig(globals);
    globals.pinsInitialized = false;
  }
  else if (server.uri() == "/submitPinSettingsJson") {
    processPinJsonResults(server, globals);
    saveConfig(globals);
    globals.pinsInitialized = false;
  }
  
  if (server.uri() == "/submitIntegrationSettings" || server.uri() == "/integrationSettings") {
    ProcessorCallback cb = BIND_PROCESSOR(settingsProcessor);
    if (ESPTemplateProcessor(server).send(String("/settings.html"), cb)) {
      Serial.println("SUCCESS");
      return;
    } else {
      Serial.println("FAIL");
      handleRoot();
    }
    return;
  }

  if (server.uri() == "/submitPinSettings" || server.uri() == "/pinSettings") {
    ProcessorCallback cb = BIND_PROCESSOR(settingsPinProcessor);
    if (ESPTemplateProcessor(server).send(String("/settings.html"), cb)) {
      Serial.println("SUCCESS");
      return;
    } else {
      Serial.println("FAIL");
      handleRoot();
    }
    return;
  }

  if (server.uri() == "/submitPinSettingsJson" || server.uri() == "/pinSettingsJson") {
    ProcessorCallback cb = BIND_PROCESSOR(settingsPinJsonProcessor);
    if (ESPTemplateProcessor(server).send(String("/PinSettings.html"), cb)) {
      Serial.println("SUCCESS");
      return;
    } else {
      Serial.println("FAIL");
      handleRoot();
    }
    return;
  }
  ProcessorCallback cb = BIND_PROCESSOR(settingsLinksProcessor);
  if (ESPTemplateProcessor(server).send(String("/settings.html"), cb)) {
    Serial.println("SUCCESS");
    return;
  } else {
    Serial.println("FAIL");
    handleRoot();
  }
}


#define RELAY_1 D5
bool relay1 = 0;
void GardenServer::handleToggleRelay1() {
  relay1 = !relay1;
  server.send(200, "text/plain", String("Relay 1 state: ") + relay1);
  digitalWrite(RELAY_1, relay1);
}

void GardenServer::handleGenericArgs() { //Handler
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

void GardenServer::handleSpecificArg() { 
  String message = "";
  
  if (server.arg("Temperature")== ""){     //Parameter not found
    message = "Temperature Argument not found";
  } else {     //Parameter found
    message = "Temperature Argument = ";
    message += server.arg("Temperature");     //Gets the value of the query parameter
  }

  server.send(200, "text/plain", message);
}

void GardenServer::handleSensor() {
  String webString;
  for (int i = 0; i < NUM_PINS; i++) {
    Pin& p = globals.pins[i];
    switch (p.type) {
      case PinType_Input_TempSensorDHT11:
      case PinType_Input_TempSensorDHT22:
        if (p.data.tempData.status) {
          webString += String("\nFailed to read sensor data on pin ") + i + ":" + String(p.name) + "\n";
        } else {
          webString += String("\n== Pin ") + i + ": " + p.name + " ==\n";
          webString += " Temperature: " + String(p.data.tempData.temp_f,2) + " F\n";
          webString += " Humidity:    " + String(p.data.tempData.humidity, 2) + "%\n";
        }
      break;
    }
  }
  server.send(200, "text/plain", webString);
}

void GardenServer::handleNotFound(){
  if (server.uri().startsWith("/submit")) {
    Serial.println("Setting submission detected.");
    handleSettings();
    return;
  }
  
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
