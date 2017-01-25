#ifndef ESP_TEMPLATE_PROCESSOR_H
#define ESP_TEMPLATE_PROCESSOR_H

#include <ESP8266WebServer.h>
#include <FS.h>
#include <stack>

typedef std::function<String(const String&)> ProcessorCallback;

class ESPTemplateProcessor {
  public:
    ESPTemplateProcessor(ESP8266WebServer& _server) :
      server(_server)
    {
    }

    bool send(const String& filePath, ProcessorCallback& processor)
    {
      std::stack<FileState> files;

      FileState state;
      state.path = filePath;
      state.offset = 0;
      files.push(state);
      
      // Send header.
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.sendHeader("Content-Type","text/html",true);
      server.sendHeader("Cache-Control","no-cache");
      server.send(200);

      // Process!
      static const uint16_t MAX = 1024;
      String buffer;
      int bufferLen = 0;
      String keyBuffer;
      int val;
      char ch;
      while (files.size() > 0) {
        FileState currentState = files.top();
        files.pop();
        Serial.println("Grabbing next file: " + currentState.path);
        
        if(!SPIFFS.exists(currentState.path)) {
          Serial.print("Cannot process '"); Serial.print(currentState.path); Serial.println("': Does not exist.");
          return false;
        }

        Serial.println("Seeking: " + String(currentState.offset, DEC));
        File currentFile = SPIFFS.open(currentState.path, "r");
        currentFile.seek(currentState.offset, SeekSet);
        
        if (!currentFile) {
          Serial.print("Cannot process "); Serial.print(currentState.path); Serial.println(": Does not exist.");
          return false;
        }

        while ((val = currentFile.read()) != -1) {
          ch = char(val);
          
          bool enterExpansion = false;
          // Lookup expansion if there are 3 in a row.
          if (ch == '%') {
            String looking = "%";
            int len = 2;
            while (len-- > 0 && (val = currentFile.read()) != -1) {
              looking += char(val);
            }

            if (looking == "%%%") {
              enterExpansion = true;
            } else {
              buffer += looking;
            }
          } else {
            bufferLen++;
            buffer += ch;
            if (bufferLen >= MAX) {
              server.sendContent(buffer);
              bufferLen = 0;
              buffer = "";
            }
          }

          if (enterExpansion) {
            // Clear out buffer.
            server.sendContent(buffer);
            buffer = "";
            bufferLen = 0;
  
            // Process substitution.
            keyBuffer = "";
            bool found = false;
            while (!found && (val = currentFile.read()) != -1) {
              ch = char(val);
              if (ch == '%') {
                found = true;
              } else {
                keyBuffer += ch;
              }
            }
            
            // Check for bad exit.
            if (val == -1 && !found) {
              Serial.print("Cannot process "); Serial.print(currentState.path); Serial.println(": Unable to parse.");
              return false;
            }

            // Include file expansion.
            if (keyBuffer.startsWith("INCLUDE:")) {
              Serial.println("Switching to file:" + keyBuffer.substring(8));
              currentState.offset = currentFile.position();
              currentFile.close();
              files.push(currentState);
              
              FileState state;
              state.path = keyBuffer.substring(8);
              state.offset = 0;
              files.push(state);
              break;
            } else {
              // Get substitution
              String processed = processor(keyBuffer);
              //Serial.print("Lookup '"); Serial.print(keyBuffer); Serial.print("' received: "); Serial.println(processed);
              server.sendContent(processed);
            }
          }
        }

        currentFile.close();
      }
  
      if (val == -1) {
        server.sendContent(buffer);
        server.sendContent("");
        return true;
      } else {
        Serial.print("Failed to process '"); Serial.print(filePath); Serial.println("': Didn't reach the end of the file.");
        return false;
      }
    }

  private:
    ESP8266WebServer &server;

    struct FileState {
      String path;
      uint32_t offset;
    };
};

#endif
