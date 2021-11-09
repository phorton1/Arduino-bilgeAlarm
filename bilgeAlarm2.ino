
#include <myDebug.h>
#include "myWebServer.h"
#include <SPIFFS.h>

uint8_t buf[255];

void setup()
{
    Serial.begin(115200);
    delay(1000);
    display(0,"bilgeAlarm2.ino started...",0);

    if (!SPIFFS.begin())
    {
        my_error("Could not initialize SPIFFS",0);
    }

    init_WiFi();
    web_server.begin();
}


void loop()
{
    web_server.handle();
    delay(1);
}
