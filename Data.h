#ifndef DATA_H
#define DATA_H

const char SettingsHeader[] PROGMEM = "<html>\
  <head>\
    <title>Settings</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <form action='/submit' method='POST'>\n\
";

const char SettingsFooter[] PROGMEM = "    <input type='submit' value='Submit'>\
</form>\
  </body>\
</html>";

#endif
