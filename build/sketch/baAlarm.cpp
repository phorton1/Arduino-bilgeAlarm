//-----------------------------------
// baAlarm.cpp - see baExterns.h
//-----------------------------------
#include "bilgeAlarm.h"
#include <myIOTLog.h>

#define DEBUG_ALARM   0

#define CHIRP_TIME              30
#define NUM_CRITICAL_CHIRPS     5
#define TIME_BETWEEN_CHIRPS     300
#define TIME_BETWEEN_ALARMS     9000        // to be parameterized



static void chirp()
{
    digitalWrite(PIN_ALARM,1);
    delay(CHIRP_TIME);
    digitalWrite(PIN_ALARM,0);
}


static void alarmTask(void *param)
    // !DEBUG_ALARMS == very small task stack!!
    // Be sure to bracket LOG calls in #if DEBUG_ALARMS
{
    delay(3000);

    static bool alarm_on = false;
    static uint32_t alarm_time = 0;
    static uint32_t last_alarm_state = 0;

    while (1)
    {
        vTaskDelay(1);
        uint32_t now = millis();
        uint32_t alarm_state = bilgeAlarm::getAlarmState();
        if (last_alarm_state != alarm_state)
        {
            #if DEBUG_ALARMS
                LOGD("alarm_state_changed to 0x%02x",_alarm_state);
            #endif

            last_alarm_state = alarm_state;

            alarm_time = 0;
            alarm_on =
                (alarm_state & ALARM_STATE_ANY) &&
                !(alarm_state & ALARM_STATE_SUPPRESSED);

            if (!alarm_on)
                digitalWrite(PIN_ALARM,0);
            else if (alarm_state & ALARM_STATE_EMERGENCY)
                digitalWrite(PIN_ALARM,alarm_on);
        }
        else if (alarm_on &&
            alarm_state < ALARM_STATE_EMERGENCY &&
            now > alarm_time + TIME_BETWEEN_ALARMS)
        {
            alarm_time = now;
            if (alarm_state & ALARM_STATE_CRITICAL)
            {
                for (int i=0; i<NUM_CRITICAL_CHIRPS; i++)
                {
                    chirp();
                    delay(TIME_BETWEEN_CHIRPS);
                }
            }
            else
            {
                chirp();
            }
        }
    }   // while (1)
}



void startAlarm()
{
    LOGI("starting alarmTask");
    xTaskCreate(alarmTask,
        "alarmTask",
        DEBUG_ALARM ? 4096 : 1024,  // very small stack unless debuggin
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL);
}


