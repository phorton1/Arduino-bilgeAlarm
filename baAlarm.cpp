//-----------------------------------
// baAlarm.cpp - see baExterns.h
//-----------------------------------
#include "bilgeAlarm.h"
#include "baAlarm.h"
#include <myIOTLog.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG_ALARM   0
#define DEBUG_PIXEL   0

#if DEBUG_ALARM
    #define DBG_ALARM(...)        LOGD(__VA_ARGS__)
    #if DEBUG_PIXEL
        #define DBG_PIXEL(...)    LOGD(__VA_ARGS__)
    #else
        #define DBG_PIXEL(...)
    #endif
#else
    #define DBG_ALARM(...)
    #define DBG_PIXEL(...)
#endif

#define NUM_CRITICAL            5

#define CHIRP_TIME              10
#define FLASH_TIME              300
#define TIME_BETWEEN_CYCLES     600
#define TIME_BETWEEN_ALARMS     8000

//--------------------------
// pixels
//--------------------------
// need a brightness setting setBrightness(uint8_t);

#define PIXEL_PUMP1         0
#define PIXEL_RELAY         1
#define PIXEL_PUMP2         2
#define PIXEL_ALARM         3
#define PIXEL_EXTERN        4
#define NUM_PIXELS          5

// dimmed colors are built into constants

#define MY_LED_BLACK    0x000000
#define MY_LED_RED      0xff0000
#define MY_LED_GREEN    0x00ff00
#define MY_LED_BLUE     0x0000ff
#define MY_LED_CYAN     0x00ffff
#define MY_LED_YELLOW   0xffff00
#define MY_LED_MAGENTA  0xff00ff
#define MY_LED_WHITE    0xffffff

Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LED_DATA);
static int startup_pixel_num = 0;
static int int_pixel_bright = DEFAULT_LED_BRIGHT;
static int ext_pixel_bright = DEFAULT_EXT_LED_BRIGHT;

static uint32_t pixel_color[NUM_PIXELS];
static uint32_t last_color[NUM_PIXELS];
static uint32_t external_color;



static void showPixels()
{
    bool show = false;
    DBG_PIXEL("showPixels() bright=%d,%d",int_pixel_bright,ext_pixel_bright);
    proc_entry();
    for (int i=0; i<NUM_PIXELS; i++)
    {
        uint32_t color = pixel_color[i];
        // if (last_color[i] != color)
        // commented this out so that showPixels reflect brightness changes
        // at small cost in performance
        {
            last_color[i] = color;
            DBG_PIXEL("color(%d)=0x%06x",i,color);
            proc_entry();

            int bright = int_pixel_bright;
            if (i>=PIXEL_EXTERN)
                bright = ext_pixel_bright;
            uint32_t new_color = 0;
            for (int j=0; j<3; j++)
            {
                uint32_t byte = (color >> j*8) & 0xff;
                uint32_t new_byte = (byte * bright + 255) / 256;
                DBG_PIXEL("byte=0x%02x new_byte=0x%02x",byte,new_byte);
                new_color |= (new_byte << j*8);
            }

            proc_leave();
            pixels.setPixelColor(i,new_color);
            show = true;
        }
    }
    proc_leave();
    if (show)
        pixels.show();
}

static void clearPixels()
{
    for (int i=0; i<NUM_PIXELS; i++)
    {
        pixel_color[i] = MY_LED_BLACK;
    }
    showPixels();
}

static void setPixel(int i, uint32_t color)
{
    pixel_color[i] = color;
}


void initPixels()
{
    pixels.begin();
    pixels.setBrightness(255);
}

void setPixelBright(bool external, uint8_t val)
{
    DBG_PIXEL("setPixelBright(%d,%d)",external,val);

    if (external)
        ext_pixel_bright = val;
    else
        int_pixel_bright = val;
    showPixels();
}


void showIncSetupPixel()	// for startup sequence
{
    static int pixel_num = 0;
    setPixel(startup_pixel_num++,MY_LED_MAGENTA);
    showPixels();
}

void bilgeAlarm::showIncSetupProgress()
    // called by myIOTDevice 3 times during setup
{
    showIncSetupPixel();
}


//--------------------------
// baAlarm
//--------------------------



static void alarmTask(void *param)
    // !DEBUG_ALARMS == very small task stack!!
    // Be sure to bracket LOG calls in #if DEBUG_ALARMS
{
    vTaskDelay(500 / portTICK_PERIOD_MS);
    DBG_ALARM("starting alarmTask loop on core(%d)",xPortGetCoreID());

    showIncSetupPixel();        // 5 (external)
    vTaskDelay(500 / portTICK_PERIOD_MS);
    clearPixels();
    showPixels();

    static uint32_t last_state = 0;
    static uint32_t last_alarm_state = 0;
    static uint32_t last_external_color = 0;

    static bool alarm_on = 0;
    static uint32_t alarm_time = 0;
    static uint32_t cycle_time = 0;
    static int cycle_num = 0;
    static int num_cycles = 0;

    static uint32_t chirp_off_time = 0;
    static uint32_t flash_off_time = 0;
    static uint32_t just_flash_time = 0;

    while (1)
    {
        #define ALARM_REFRESH 50
        vTaskDelay(ALARM_REFRESH / portTICK_PERIOD_MS);

        if (1)
        {
            uint32_t now = millis();
            uint32_t state = bilgeAlarm::getState();
            uint32_t alarm_state = bilgeAlarm::getAlarmState();

            bool show_pixels = false;

            state &= (STATE_PUMP1_ON | STATE_PUMP2_ON | STATE_RELAY_ON);
            if (last_state != state)
            {
                last_state = state;
                DBG_ALARM("alarmTask:state changed to 0x%04x",state);
                setPixel(PIXEL_PUMP1,state & STATE_PUMP1_ON ? MY_LED_BLUE  : MY_LED_BLACK);
                setPixel(PIXEL_PUMP2,state & STATE_PUMP2_ON ? MY_LED_RED   : MY_LED_BLACK);
                setPixel(PIXEL_RELAY,state & STATE_RELAY_ON ? MY_LED_GREEN : MY_LED_BLACK);
                external_color =
                    state & STATE_PUMP2_ON ? MY_LED_RED :
                    state & STATE_RELAY_ON ? MY_LED_GREEN :
                    state & STATE_PUMP1_ON ? MY_LED_BLUE :
                    MY_LED_BLACK;
                show_pixels = true;
            }

            if (last_external_color != external_color)
            {
                last_external_color = external_color;
                setPixel(PIXEL_EXTERN,external_color);
                show_pixels = true;
            }

            // the ALARM_LED stays on in an alarm state,
            // but the alarm itself chirps and the external led flashes
            // the external led flashes incessintally during an alarm, wheras
            // the alarm is full on.

            if (last_alarm_state != alarm_state)
            {
                last_alarm_state = alarm_state;
                DBG_ALARM("alarmTask:alarm_state changed to 0x%02x",alarm_state);

                alarm_on =
                    (alarm_state & ALARM_STATE_ANY) &&
                    !(alarm_state & ALARM_STATE_SUPPRESSED);
                num_cycles =
                    alarm_state & ALARM_STATE_EMERGENCY ? -1 :
                    alarm_state & ALARM_STATE_CRITICAL ? NUM_CRITICAL : 1;
                cycle_num = 0;

                digitalWrite(PIN_ALARM,alarm_on);
                setPixel(PIXEL_ALARM,alarm_state?MY_LED_MAGENTA:MY_LED_BLACK);
                setPixel(PIXEL_EXTERN,alarm_on?MY_LED_MAGENTA:external_color);

                just_flash_time = 0;
                alarm_time = alarm_on && num_cycles != -1 ? now + TIME_BETWEEN_ALARMS : 0;
                cycle_time = alarm_on && num_cycles != -1 ? now + TIME_BETWEEN_CYCLES : 0;
                chirp_off_time = alarm_on && num_cycles != -1 ? now + CHIRP_TIME : 0;
                flash_off_time = alarm_on ? now + FLASH_TIME  : 0;
                show_pixels = true;

                DBG_ALARM("alarm_on(%d) num_cycles(%d) alarm_time=%d cycle_time=%d chirp_off_time=%d flash_off_time=%d", alarm_on,num_cycles,alarm_time,cycle_time,chirp_off_time,flash_off_time);
            }

            if (chirp_off_time && now > chirp_off_time)
            {
                digitalWrite(PIN_ALARM,0);
                // DBG_ALARM("chirp_off");
                chirp_off_time = 0;
            }
            if (flash_off_time && now > flash_off_time)
            {
                setPixel(PIXEL_EXTERN,external_color);
                flash_off_time = 0;
                if (num_cycles == -1)
                    just_flash_time = now + TIME_BETWEEN_CYCLES - FLASH_TIME;
                DBG_ALARM("flash_off flash_off_time=%d",flash_off_time);
                show_pixels = true;
            }
            if (just_flash_time && now > just_flash_time)
            {
                just_flash_time = 0;
                setPixel(PIXEL_EXTERN,MY_LED_MAGENTA);
                flash_off_time = now + FLASH_TIME;
                show_pixels = true;
            }
            if (cycle_time && now > cycle_time)
            {
                cycle_num++;
                if (cycle_num >= num_cycles)
                {
                    cycle_num = 0;
                    cycle_time = 0;
                    DBG_ALARM("cycle ended");
                }
                else
                {
                    digitalWrite(PIN_ALARM,1);
                    setPixel(PIXEL_EXTERN,MY_LED_MAGENTA);
                    cycle_time = now + TIME_BETWEEN_CYCLES;
                    chirp_off_time = now + CHIRP_TIME;
                    flash_off_time = now + FLASH_TIME;;
                    DBG_ALARM("cycle_ended(%d) cycle_time=%d chirp_off_time=%d flash_off_time=%d",cycle_num,cycle_time,chirp_off_time,flash_off_time);
                    show_pixels = true;
                }
            }
            if (alarm_time && now > alarm_time)
            {
                digitalWrite(PIN_ALARM,1);
                setPixel(PIXEL_EXTERN,MY_LED_MAGENTA);
                alarm_time = now + TIME_BETWEEN_ALARMS;
                cycle_num = 0;
                cycle_time = now + TIME_BETWEEN_CYCLES;
                chirp_off_time = now + CHIRP_TIME;
                flash_off_time = now + FLASH_TIME;
                DBG_ALARM("next_alarm() alarm_time=%d cycle_time=%d chirp_off_time=%d flash_off_time=%d", alarm_time,cycle_time,chirp_off_time,flash_off_time);
                show_pixels = true;
            }

            if (show_pixels)
                showPixels();

        }   // if ()
    }   // while (1)
}



void startAlarm()
{
    LOGI("starting alarmTask");
    xTaskCreate(alarmTask,
        "alarmTask",
        DEBUG_ALARM ? 4096 : 1024,  // very small stack unless debugging
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL);
}


