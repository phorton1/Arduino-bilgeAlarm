//-----------------------------------
// bilgeAlarm.ino
//-----------------------------------
// Interesting devlopement regarding the history in RAM.
// I moved it to the RTC_NOINIT_ATTR memory, and separately
// implemented a Value for the RESET_COUNT in factoryReset().
//
// The history is cleared only upon a POWERON_RESET,
// and is now persistent through soft-reboots.

// Note that recompiling from the Arduino IDE does a POWERON_RESET,
// though OTA updates don't ... so, the history in RAM will survive
// a OTA update.  Perhaps I can figure out a way to have the Arduino
// IDE NOT do a POWERON_RESET.
//
// Modifying C:\Users\Patrick\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6
// platforms.txt to try changing "--after  hard_reset". "soft_reset" is not supported.
// "no_reset" with the serial port connected causes, I think, the com port to reboot it.
// Without the serial port, the ESP32 presumably stays in bootloader mode, as, even
// though the LCD is still displaying the last message, the buttons don't work and
// there is no indication of rebooting.     So I can't run any commands like "reboot"
// ...  hmmmm ... if I press the reset button, it's another hard power on reset.

// NO CONSOLE:
//      compile with "hard_reset":  RTCWDT_RTC_RESET
//      pressing the reset button:  RTCWDT_RTC_RESET
//      powering up:                RTCWDT_RTC_RESET
//      reboot from menu:           SW_CPU_RESET
// CONSOLE:
//      start console:              POWERON_RESET
//      compile with "hard_reset":  POWERON_RESET
//      reboot from menu:           SW_CPU_RESET

// We (must) reinitialize the RTC memory if either
// RTCWDT_RTC_RESET or POWERON_RESET, or else it is random.

// The history SHALL write a database to the SPIFFS file system,
// which appends each run as a 64 bit (8 byte) record to the file.
// Upon bilgeAlarm::onInitRTCMemeory, which calls initHistoryRTCMemory,'
// the last N entries from that file shall be read into memory.
//
// I am currently using 416.9K of 1.37M SPIFFS total, which leaves about 800K,
// or enough room to store 100,000 runs ... but for safety I would probably
// keep 10,000 with a rotation scheme.

// And/or use the SDCard if it's available.

// But first I want to rename baRuns to baHistory and make it a proper class.
// Checking in Persistent Memory WIP





#include "bilgeAlarm.h"
#include "baExterns.h"      // for pixels, maybe SDCAR
#include <myIOTLog.h>

#ifdef WITH_SD
    #define INIT_SD_EARLY
#endif


//--------------------------------
// main
//--------------------------------


void setup()
{
    Serial.begin(115200);
    delay(1000);

    initPixels();

    bilgeAlarm::setDeviceType(BILGE_ALARM);
    bilgeAlarm::setDeviceVersion(BILGE_ALARM_VERSION);

    // init the SD Card in early derived device
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

    #ifdef WITH_SD
    #ifdef INIT_SD_EARLY
        LOGD("sd_ok=%d",sd_ok);
    #endif
    #endif

    bilge_alarm = new bilgeAlarm();
    bilge_alarm->setup();

    showIncSetupPixel();    // 4 (external)
        // 1,2,3 is IOTDevice.
        // 4 is probably this one
        // and 5 is probably the alarmTask starting up
        // then they go off

    LOGU("bilgeAlarm.ino setup() finished",0);
}



void loop()
{
    bilge_alarm->loop();

    #if TEST_VRSION
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
