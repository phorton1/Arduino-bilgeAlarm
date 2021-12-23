//-----------------------------------
// bilgeAlarm2.ino
//-----------------------------------
#include "bilgeAlarm.h"
#include <myIOTLog.h>

#ifdef WITH_SD
    #define INIT_SD_EARLY
    #ifdef INIT_SD_EARLY
        #include <SD.h>
    #endif
#endif


//--------------------------------
// main
//--------------------------------


void setup()
{
    Serial.begin(115200);
    delay(1000);

    bilge_alarm = new bilgeAlarm();
    bilge_alarm->init();

    LOGU("bilgeAlarm2.ino setup() started");

    #ifdef WITH_SD
    #ifdef INIT_SD_EARLY
        delay(200);
        bool sd_ok = SD.begin(5);     // 5
        LOGI("sd_ok=%d",sd_ok);
    #endif
    #endif

    bilge_alarm->setup();

    LOGI("%s version=%s IOTVersion=%s",
         bilge_alarm->getName().c_str(),
         bilge_alarm->getVersion(),
         IOT_DEVICE_VERSION);

    LOGU("bilgeAlarm2.ino setup() finished",0);
}



void loop()
{
    bilge_alarm->loop();

    #if 1
        if (bilge_alarm->getBool(ID_DEMO_MODE))
        {
            uint32_t now = millis();
            static uint32_t toggle_led = 0;
            if (now > toggle_led + 2000)
            {
                toggle_led = now;
                bool led_state = bilge_alarm->getBool(ID_ONBOARD_LED);
                bilge_alarm->setBool(ID_ONBOARD_LED,!led_state);
            }
        }
    #endif
}
