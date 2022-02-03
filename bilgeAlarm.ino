//-----------------------------------
// bilgeAlarm.ino
//-----------------------------------

#include "bilgeAlarm.h"
#include "baAlarm.h"      // for showIncSetupPixel()
#include <myIOTLog.h>

#ifdef WITH_SD
    #define INIT_SD_EARLY
#endif


//--------------------------------
// main
//--------------------------------


void setup()
{
    initPixels();

    Serial.begin(115200);
    delay(1000);

    bilgeAlarm::setDeviceType(BILGE_ALARM);
    bilgeAlarm::setDeviceVersion(BILGE_ALARM_VERSION);

    // init the SD Card early in derived device
    // due to it's wonky SPI behavior, and so that
    // logfile can begin immediately.

    #ifdef WITH_SD
    #ifdef INIT_SD_EARLY
        bool sd_ok = bilgeAlarm::initSDCard();
    #endif
    #endif

    LOGU("");
    LOGU("");
    LOGU("bilgeAlarm.ino setup() started on core(%d)",xPortGetCoreID());

    showIncSetupPixel();    // 0

    #ifdef WITH_SD
    #ifdef INIT_SD_EARLY
        LOGD("sd_ok=%d",sd_ok);
    #endif
    #endif

    bilge_alarm = new bilgeAlarm();
    bilge_alarm->setup();


        // 1,2,3 is IOTDevice.
        // 4 is probably this one
        // and 5 is probably the alarmTask starting up
        // then they go off

    LOGU("bilgeAlarm.ino setup() finished",0);
}



void loop()
{
    bilge_alarm->loop();

    #if DEMO_MODE
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
