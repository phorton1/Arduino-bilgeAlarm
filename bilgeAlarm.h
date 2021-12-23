//-----------------------------------
// bilgeAlarm.h
//-----------------------------------
#pragma once

#include <myIOTDevice.h>

//------------------------
// pins
//------------------------

#define PIN_ONBOARD_LED     2
#define PIN_OTHER_LED       33      // for old test ui - not on actual bilgeAlarm board

// PIN_SD_CS = 5
// PIN_MOSI  = 23
// PIN_SCLK  = 18
// PIN_MISO  = 19

#define PIN_BUTTON1         13
#define PIN_BUTTON2         14
#define PIN_BUTTON3         33
#define PIN_BUTTON4         32
#define PIN_PUMP1_ON        36
#define PIN_PUMP2_ON        39
#define PIN_12V_IN          34
#define PIN_5V_IN           35

// PIN_SDA = 21
// PIN_SCL = 22

#define PIN_RELAY           15
#define PIN_ALARM           12
#define PIN_LED_DATA        4


//------------------------
// ids
//------------------------

#define ID_STATE            "STATE"
#define ID_ALARM_STATE      "ALARM_STATE"

#define ID_DISABLED         "DISABLED"
#define ID_BACKLIGHT_SECS   "BACKLIGHT_SECS"
#define ID_ERR_RUN_TIME     "ERR_RUN_TIME"
#define ID_CRIT_RUN_TIME    "CRIT_RUN_TIME"
#define ID_ERR_PER_HOUR     "ERR_PER_HOUR"
#define ID_ERR_PER_DAY      "ERR_PER_DAY"
#define ID_EXTRA_RUN_TIME   "EXTRA_RUN_TIME"
#define ID_EXTRA_RUN_MODE   "EXTRA_RUN_MODE"
#define ID_END_RUN_DELAY    "END_RUN_DELAY"
#define ID_RUN_EMERGENCY    "RUN_EMERGENCY"

#define ID_RELAY            "RELAY"
#define ID_ALARM            "ALARM"

#define ID_ONBOARD_LED      "ONBOARD_LED"
#define ID_OTHER_LED        "OTHER_LED"
#define ID_DEMO_MODE        "DEMO_MODE"

#define ID_LCD_LINE1        "LCD_LINE1"
#define ID_LCD_LINE2        "LCD_LINE2"


//------------------------
// states
//------------------------

#define ALARM_STATE_NONE           0x00
#define ALARM_STATE_ERROR          0x01
#define ALARM_STATE_CRITICAL       0x02
#define ALARM_STATE_EMERGENCY      0x04
#define ALARM_STATE_SUPPRESSED     0x08

#define STATE_NONE                 0x0000
#define STATE_PUMP_ON              0x0001
#define STATE_RELAY_ON             0x0002
#define STATE_RELAY_FORCE_ON       0x0004
#define STATE_RELAY_EMERGENCY      0x0008
#define STATE_EMERGENCY_PUMP_RUN   0x0010
#define STATE_EMERGENCY_PUMP_ON    0x0020
#define STATE_TOO_OFTEN_HOUR       0x0040
#define STATE_TOO_OFTEN_DAY        0x0080
#define STATE_TOO_LONG             0x0100
#define STATE_CRITICAL_TOO_LONG    0x0200


class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm();
    ~bilgeAlarm() {}

    virtual const char *getVersion() override  { return "b0.05"; }

    virtual void setup() override;
    virtual void loop() override;

private:

    static const valDescriptor m_bilge_values[];

    // vars

    static uint32_t m_state;
    static uint32_t m_alarm_state;

    static bool m_RELAY;
    static bool m_ONBOARD_LED;
    static bool m_OTHER_LED;
    static bool m_DEMO_MODE;

    static String m_lcd_line1;
    static String m_lcd_line2;

    // methods

    void handleButtons();
    void handleSensors();

    static void onRelay(const myIOTValue *desc, bool val);
    static void onLed(const myIOTValue *desc, bool val);
    static void onLcdLine(const myIOTValue *desc, const char *val);

    static void setState(uint32_t state);
    static void clearState(uint32_t state);
    static void changeState(bool on, uint32_t state)
    {
        if (on)
            setState(state);
        else
            clearState(state);
    }

    static void setAlarmState(uint32_t alarm_state);
    static void clearAlarmState(uint32_t alarm_state);
    static void changeAlarmState(bool on, uint32_t alarm_state)
    {
        if (on)
            setAlarmState(alarm_state);
        else
            clearAlarmState(alarm_state);
    }

};


extern bilgeAlarm *bilge_alarm;
