//-----------------------------------
// bilgeAlarm.h
//-----------------------------------

#pragma once

#include <myIOTDevice.h>

#define BILGE_ALARM             "bilgeAlarm"
#define BILGE_ALARM_VERSION     "b0.07"
#define BILGE_ALARM_URL			"https://github.com/phorton1/Arduino-bilgeAlarm"


#define BILGE_POWER          1
    // standard feature - show the 12V and 5V power supplies
#define HAS_LCD_LINE_VALUES  0
    // experimental feature - use VALUES for the two lines of LCD text


#define DEFAULT_LED_BRIGHT        10
#define DEFAULT_EXT_LED_BRIGHT    60

//------------------------
// pins
//------------------------

#define PIN_ONBOARD_LED     2
#define PIN_OTHER_LED       33      // for old test ui - not on actual bilgeAlarm board

// PIN_SD_CS = 5
// PIN_MOSI  = 23
// PIN_SCLK  = 18
// PIN_MISO  = 19

#define PIN_BUTTON0         13
#define PIN_BUTTON1         33
#define PIN_BUTTON2         32

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
// state variables

#define ID_STATE            "STATE"
#define ID_ALARM_STATE      "ALARM_STATE"
#define ID_TIME_LAST_RUN    "TIME_LAST_RUN"
#define ID_DUR_LAST_RUN     "DUR_LAST_RUN"
#define ID_SINCE_LAST_RUN   "SINCE_LAST_RUN"
#define ID_HOUR_CUTOFF      "HOUR_CUTOFF"
#define ID_DAY_CUTOFF       "DAY_CUTOFF"
#define ID_NUM_LAST_HOUR    "NUM_LAST_HOUR"
#define ID_NUM_LAST_DAY     "NUM_LAST_DAY"
#define ID_NUM_LAST_WEEK    "NUM_LAST_WEEK"

// power

#if BILGE_POWER
    #define ID_POWER_12V    "POWER_12V"         // float - displayed 12.1 volts
    #define ID_POWER_5V     "POWER_5V"          // float - displayed 5.1 volts
    #define ID_CALIB_12V    "CALIB_12V"         // int - calib/1000
    #define ID_CALIB_5V     "CALIB_5V"          // int - calib/1000
#endif

// commands

#define ID_SELFTEST         "SELF_TEST"             // user command to run self test
#define ID_FORCE_RELAY      "FORCE_RELAY"           // the user command (switch) to force the relay on or off
#define ID_SUPPRESS         "SUPPRESS_ALARM"        // the user command (button) to suppress the alarm sound
#define ID_CLEAR_ERROR      "CLEAR_ERROR"           // the user command (button) to clear the current error
#define ID_CLEAR_HISTORY    "CLEAR_HISTORY"         // the user command (button) to clear the run_history
    // There are a number of scenarios about handling ALARMS.
    // I press the SUPPRESS button to supress the alarm from ringing.
    // The TOO_LONG, CRITICAL_TOO_LONG, and EMERGENCY alarms are correctly cleared by CLEAR_ERROR.
    // The TOO_MANY_PER_HOUR and TOO_MANY_PER_DAY errors will be sort-of handled by CLEAR_ERROR,
    //     in that they won't re-invoke until the pump comes back on again.  If it is after an
    //     appropriate interval, the system will normalize.    Finally, one can REBOOT or
    //     use CLEAR_HISTORY to clear the historical data and the TOO_MANY_PER counters will
    //     start over.  For good measure, CLEAR_HISTORY also calls CLEAR_ERROR.
    // In any case, all of it's in the log files.
#define ID_HISTORY_LINK     "HISTORY_LINK"

// configuration

#define ID_DISABLED         "ALARMS"                // _disabled = enabled,disabled = LEDs still light, but no errors, extra runs, or other functionality will take place
#define ID_ERR_RUN_TIME     "ERR_RUN_TIME"          // off,secs - pump running this many seconds or more is an "error alarm"
#define ID_CRIT_RUN_TIME    "CRIT_RUN_TIME"         // off,secs - pump running this many seconds or more is a "critical error alarm"
#define ID_ERR_PER_HOUR     "ERR_PER_HOUR"          // off,num - pump running more than this many times per hour is an "error alarm"
#define ID_ERR_PER_DAY      "ERR_PER_DAY"           // off,secs - pump running more than this many times per day is an "error alarm"
#define ID_EXTRA_RUN_TIME   "EXTRA_RUN_TIME"        // off,secs - how long to turn on relay if pump goes on,
#define ID_EXTRA_RUN_MODE   "EXTRA_RUN_MODE"        // at_start, after_end - whether to turn on relay when pump turns on, or when it turns off
#define ID_EXTRA_RUN_DELAY  "EXTRA_RUN_DELAY"       // millis, if after_end, how long after pump goes off before we turn on relay (long debounce time)
    // The "duration" does not include any time while the relay is on.
#define ID_SENSE_MILLIS     "SENSE_MILLIS"          // millis - how often to check the pump input pins, 5 minimum 30 default
#define ID_PUMP_DEBOUNCE    "PUMP_DEBOUNCE"         // millis - how long after the pump switch changes before we read it again
#define ID_RELAY_DEBOUNCE   "RELAY_DEBOUNCE"        // millis - how long after the relay goes off before we read the pump switch again
    // We read the pump sensors every SENSOR_MILLIS milliseconds.
    // The PUMP_DEBOUNCE value is how long after it changes before we read it again (though we act upon it immediately)
    // The RELAY_DEBOUNCE time is how long after we turn the relay off before we check the pump switch again.
    //    The relay will energize the pump switch and due to inductance in the motor, the switch may appear on
    //    when it is not really on for some period of time as the motor spins down.
#define ID_SW_THRESHOLD     "SW_THRESHOLD"          // analogRead() threshold for pump switches - default==900, see logfile if this value needs to be changed
#define ID_RUN_EMERGENCY    "RUN_EMERGENCY"         // off,secs - how long to turn on the relay as long as the emergency switch turns on
    // This will run the main pump if the emergency pump turns on, and continue running it for N more seconds than the emergency pump
#define ID_LED_BRIGHT       "LED_BRIGHT"            // 0..255 main ws2812 LED brightness
#define ID_EXT_LED_BRIGHT   "EXT_LED_BRIGHT"        // 0..255 ext ws2812 LED brightness
#define ID_BACKLIGHT_SECS   "BACKLIGHT_SECS"        // off,secs = backlight will turn off if no buttons pressed after this many seconds
#define ID_MENU_SECS        "MENU_SECS"             // off,secs = will return to the MAIN SCREEN if no buttons pressed after this many seconds


// optional

#if HAS_LCD_LINE_VALUES
    #define ID_LCD_LINE1        "LCD_LINE1"
    #define ID_LCD_LINE2        "LCD_LINE2"
#endif


//------------------------
// states
//------------------------

#define ALARM_STATE_NONE           0x00
#define ALARM_STATE_ERROR          0x01
#define ALARM_STATE_CRITICAL       0x02
#define ALARM_STATE_EMERGENCY      0x04
#define ALARM_STATE_SUPPRESSED     0x08
#define ALARM_STATE_ANY            (ALARM_STATE_ERROR | ALARM_STATE_CRITICAL | ALARM_STATE_EMERGENCY)

// monadic, actual states of pump switches and relay

#define STATE_NONE                 0x0000
#define STATE_EMERGENCY            0x0001       // the emergency pump switch was turned on
#define STATE_PUMP1_ON             0x0002       // the regular pump switch is on
#define STATE_PUMP2_ON             0x0004       // the emergency pump switch is on
#define STATE_RELAY_ON             0x0008       // the relay is on

// relay on states

#define STATE_RELAY_FORCED         0x0010      // the relay was forced on or off by the user
#define STATE_RELAY_EMERGENCY      0x0020      // the relay is on due to the ID_RUN_EMERGENCY value
#define STATE_RELAY_EXTRA          0x0040      // the relay is on due to the ID_EXTRA_RUN_TIME value

#define RELAY_STATE_ANY            (STATE_RELAY_FORCED | STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA)

// error states

#define STATE_TOO_OFTEN_HOUR       0x0080       // error detected
#define STATE_TOO_OFTEN_DAY        0x0100       // error detected
#define STATE_TOO_LONG             0x0200       // error detected
#define STATE_CRITICAL_TOO_LONG    0x0400       // critical error detected



#define ALARM_ENABLED    0
#define ALARM_SILENT     1
#define ALARM_DISABLED   2


typedef struct
    // a structure containing the states used by stateMachine()
    // for asynchronous publishing during loop()
{
    uint32_t state;
    uint32_t alarm_state;
    time_t   time_last_run;
    int      since_last_run;   // time_last_run as an int
    int      dur_last_run;
    int      num_last_hour;
    int      num_last_day;
    int      num_last_week;
} bilgeAlarmState_t;



class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm();
    ~bilgeAlarm() {}

    virtual void setup() override;
    virtual void loop() override;

    static uint32_t getState()       { return m_publish_state.state; }
    static uint32_t getAlarmState()  { return m_publish_state.alarm_state; }
    static uint32_t getDisabled()    { return _disabled; }

    static void clearError();
    static void suppressAlarm();
    static void clearHistory();
    static void selfTest();
    static bool inSelfTest()         { return m_in_self_test; }

    static time_t   _hour_cutoff;
    static time_t   _day_cutoff;

private:
    friend class baHistory;
    

    static const valDescriptor m_bilge_values[];
    static bilgeAlarmState_t m_publish_state;

    // values

    static uint32_t _state;
    static uint32_t _alarm_state;
    static time_t   _time_last_run;
    static int      _since_last_run;   // time_last_run as an int
    static int      _dur_last_run;
    static int      _num_last_hour;
    static int      _num_last_day;
    static int      _num_last_week;

    static String _history_link;

    static uint32_t  _disabled;
    static int  _backlight_secs;
    static int  _menu_secs;
    static int  _err_run_time;
    static int  _crit_run_time;
    static int  _err_per_hour;
    static int  _err_per_day;
    static int  _run_emergency;
    static int  _extra_run_time;
    static int  _extra_run_mode;
    static int  _extra_run_delay;
    static int  _sense_millis;
    static int  _pump_debounce;
    static int  _relay_debounce;
    static int  _sw_threshold;

    static float _power_12v;
    static float _power_5v;
    static int   _calib_12v;
    static int   _calib_5v;

    static bool _FORCE_RELAY;

    #if HAS_LCD_LINE_VALUES
        static String _lcd_line1;
        static String _lcd_line2;
    #endif

    // working vars

    static uint32_t m_pump1_debounce_time;
    static uint32_t m_pump2_debounce_time;
    static uint32_t m_relay_delay_time;
    static uint32_t m_relay_time;
    static bool     m_in_self_test;

    // methods

    void stateMachine();
    void publishState();
    static void stateTask(void *param);
    void checkPower();

    static void setState(uint32_t state);
    static void addState(uint32_t state);
    static void clearState(uint32_t state);

    static void setAlarmState(uint32_t alarm_state);
    static void addAlarmState(uint32_t alarm_state);
    static void clearAlarmState(uint32_t alarm_state);

    static bool debounceSwitch(int pump_num, uint32_t now, int pin, bool was_on, uint32_t *p_debounce_time);

    static void onForceRelay(const myIOTValue *desc, bool val);
    static void onDisabled(const myIOTValue *desc, bool val);
    static void onLedBright(const myIOTValue *desc, bool val);
    virtual void onValueChanged(const myIOTValue *value, valueStore from=VALUE_STORE_PROG) override;
    virtual String onCustomLink(const String &path,  const char **mime_type) override;
    // virtual void onInitRTCMemory() override;

    #if WITH_AUTO_REBOOT
        virtual bool okToAutoReboot() override;
    #endif

    #if HAS_LCD_LINE_VALUES
        static void onLcdLine(const myIOTValue *desc, const char *val);
    #endif

    virtual void showIncSetupProgress() override;

};


extern bilgeAlarm *bilge_alarm;
