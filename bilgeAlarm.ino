//-----------------------------------
// bilgeAlarm.ino
//-----------------------------------

#include "bilgeAlarm.h"
#include "baAlarm.h"      // for showIncSetupPixel()
#include <myIOTLog.h>

#if WITH_SD
    #define INIT_SD_EARLY 1
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

    #if WITH_SD
    #if INIT_SD_EARLY
        bool sd_ok = bilgeAlarm::initSDCard();
    #endif
    #endif

    LOGU("");
    LOGU("");
    LOGU("bilgeAlarm.ino setup() started on core(%d)",xPortGetCoreID());

    showIncSetupPixel();    // 0

    #if WITH_SD
    #if INIT_SD_EARLY
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
}
