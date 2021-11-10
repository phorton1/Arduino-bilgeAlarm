
#include <myDebug.h>
#include "myWebServer.h"
#include <SPIFFS.h>

#include <WiFi.h>
#include <time.h>


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

    if ((WiFi.getMode() == WIFI_STA))
    {
        const char* ntpServer = "pool.ntp.org";
        const long  gmtOffset_sec = -5 * 60 * 60;
        const int   daylightOffset_sec = 0;

        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    }
    web_server.begin();
}


void loop()
{
    web_server.handle();
    delay(1);

    // %A	returns day of week
    // %B	returns month of year
    // %d	returns day of month
    // %Y	returns year
    // %H	returns hour
    // %M	returns minutes
    // %S	returns seconds


    uint32_t now = millis();
    static uint32_t last_time = 0;
    if (now > last_time + 30000)
    {
        last_time = now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
        }
        else
        {
            Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        }
    }
}
