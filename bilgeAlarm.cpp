//-----------------------------------
// bilgeAlarm.cpp
//-----------------------------------
#include "bilgeAlarm.h"
#include <myIOTLog.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG_STATES  1
#define DEBUG_ALARM   0


#define LCD_LINE_LEN   16

#define BUTTON_CHECK_TIME  30
#define UI_UPDATE_TIME     30

//--------------------------------
// bilgeAlarm definition
//--------------------------------

#define DEFAULT_DISABLED            0          // enabled,disabled
#define DEFAULT_BACKLIGHT_SECS      0          // off,secs

#define DEFAULT_ERR_RUN_TIME        10         // off,secs
#define DEFAULT_CRIT_RUN_TIME       30         // off,secs
#define DEFAULT_ERR_PER_HOUR        4          // off,num
#define DEFAULT_ERR_PER_DAY         20         // off,secs

#define DEFAULT_EXTRA_RUN_TIME      5          // off,secs
#define DEFAULT_EXTRA_RUN_MODE      1          // at_start, after_end
#define DEFAULT_EXTRA_RUN_DELAY     1000       // millis after pump goes off to start after_end

#define DEFAULT_SENSE_MILLIS        30         // millis
#define DEFAULT_PUMP_DEBOUNCE       500        // millis
#define DEFAULT_RELAY_DEBOUNCE      2000       // millis

#define DEFAULT_RUN_EMERGENCY       30         // off,secs


// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
    ID_STATE,
    ID_ALARM_STATE,
    ID_SUPPRESS,
    ID_CLEAR_ERROR,
    ID_DISABLED,
    ID_LCD_LINE1,
    ID_LCD_LINE2,
#ifdef WITH_POWER
    ID_DEVICE_VOLTS,
    ID_DEVICE_AMPS,
#endif
    ID_FORCE_RELAY,
    ID_ONBOARD_LED,
    ID_OTHER_LED,
    ID_DEMO_MODE,
    ID_REBOOT,
    0
};

// shat shoes up on the "device" UI tab

static valueIdType device_items[] = {
    ID_DEMO_MODE,
    ID_DISABLED,
    ID_BACKLIGHT_SECS,
    ID_ERR_RUN_TIME,
    ID_CRIT_RUN_TIME,
    ID_ERR_PER_HOUR,
    ID_ERR_PER_DAY,
    ID_RUN_EMERGENCY,
    ID_EXTRA_RUN_TIME,
    ID_EXTRA_RUN_MODE,
    ID_EXTRA_RUN_DELAY,
    ID_SENSE_MILLIS,
    ID_PUMP_DEBOUNCE,
    ID_RELAY_DEBOUNCE,
    0
};


// enum strings

static enumValue alarmStates[] = {
    "ERROR",
    "CRITICAL",
    "EMERGENCY",
    "SUPPRESSED",
    0};
static enumValue systemStates[] = {
    "PUMP1_ON",
    "PUMP2_ON",
    "RELAY_ON",
    "EMERGENCY",
    "TOO_OFTEN_HOUR",
    "TOO_OFTEN_DAY",
    "TOO_LONG",
    "CRIT_LONG",
    "RELAY_FORCED",
    "RELAY_EMERGENCY",
    "RELAY_EXTRA",
    0};


// value descriptors for bilgeAlaram

const valDescriptor bilgeAlarm::m_bilge_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,   VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,                     NULL,   "bilgeAlarm" },        // override base class element

    { ID_SUPPRESS,         VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                     (void *) suppressError },
    { ID_CLEAR_ERROR,      VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                     (void *) clearError },

    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_READONLY,   (void *) &_state,         NULL,   { .enum_range = { 4, systemStates }} },
    { ID_ALARM_STATE,      VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_alarm_state,   NULL,   { .enum_range = { 4, alarmStates }} },

    { ID_DISABLED,         VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_disabled,      (void *) onDisabled,   { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_backlight_secs, NULL,  { .int_range = { DEFAULT_BACKLIGHT_SECS,    0,  3600}}  },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_run_time,   NULL,  { .int_range = { DEFAULT_ERR_RUN_TIME,      0,  3600}}  },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_crit_run_time,  NULL,  { .int_range = { DEFAULT_CRIT_RUN_TIME,     0,  3600}}  },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_per_hour,   NULL,  { .int_range = { DEFAULT_ERR_PER_HOUR,      0,  255}}   },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_per_day,    NULL,  { .int_range = { DEFAULT_ERR_PER_DAY,       0,  255}}   },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_run_emergency,  NULL,  { .int_range = { DEFAULT_RUN_EMERGENCY,     0,  3600}}  },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_time, NULL,  { .int_range = { DEFAULT_EXTRA_RUN_TIME,    0,  3600}}  },
    { ID_EXTRA_RUN_MODE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_mode, NULL,  { .int_range = { DEFAULT_EXTRA_RUN_MODE,    0,  1}}     },
    { ID_EXTRA_RUN_DELAY,  VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_delay,NULL,  { .int_range = { DEFAULT_EXTRA_RUN_DELAY,   5,  30000}} },
    { ID_SENSE_MILLIS,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_sense_millis,   NULL,  { .int_range = { DEFAULT_SENSE_MILLIS,      5,  30000}} },
    { ID_PUMP_DEBOUNCE,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pump_debounce,  NULL,  { .int_range = { DEFAULT_PUMP_DEBOUNCE,     5,  30000}} },
    { ID_RELAY_DEBOUNCE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_relay_debounce, NULL,  { .int_range = { DEFAULT_RELAY_DEBOUNCE,    5,  30000}} },

    { ID_FORCE_RELAY,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_FORCE_RELAY,    (void *) onForceRelay },
    { ID_ONBOARD_LED,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_ONBOARD_LED,    (void *) onLed, },
    { ID_OTHER_LED,        VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_OTHER_LED,      (void *) onLed, },
    { ID_DEMO_MODE,        VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_DEMO_MODE,       NULL,          },

    { ID_LCD_LINE1,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line1,      (void *) onLcdLine },
    { ID_LCD_LINE2,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line2,      (void *) onLcdLine },
};

#define NUM_BILGE_VALUES (sizeof(m_bilge_values)/sizeof(valDescriptor))


// values

uint32_t bilgeAlarm::_state;
uint32_t bilgeAlarm::_alarm_state;

bool bilgeAlarm::_disabled;
int  bilgeAlarm::_backlight_secs;
int  bilgeAlarm::_err_run_time;
int  bilgeAlarm::_crit_run_time;
int  bilgeAlarm::_err_per_hour;
int  bilgeAlarm::_err_per_day;
int  bilgeAlarm::_run_emergency;
int  bilgeAlarm::_extra_run_time;
int  bilgeAlarm::_extra_run_mode;
int  bilgeAlarm::_extra_run_delay;
int  bilgeAlarm::_sense_millis;
int  bilgeAlarm::_pump_debounce;
int  bilgeAlarm::_relay_debounce;

bool bilgeAlarm::_FORCE_RELAY;
bool bilgeAlarm::_ONBOARD_LED;
bool bilgeAlarm::_OTHER_LED;
bool bilgeAlarm::_DEMO_MODE;

String bilgeAlarm::_lcd_line1;
String bilgeAlarm::_lcd_line2;


// working vars

uint32_t bilgeAlarm::m_pump1_debounce_time;
uint32_t bilgeAlarm::m_pump2_debounce_time;
uint32_t bilgeAlarm::m_relay_delay_time;
uint32_t bilgeAlarm::m_relay_time;
bool     bilgeAlarm::m_suppress_next_after;

uint32_t bilgeAlarm::m_start_duration;
    // The "duration" that pump1 is "on"
    // INCLUDES the "extra_time" if EXTRA_MODE(0) "at_start", so
    // the ERR_RUN_TIME and CRIT_RUN_TIME values must be larger than EXTRA_RUN_TIME
    // if using EXTRA_RUN_MODE(0) "at_start"



// globals

bilgeAlarm *bilge_alarm = NULL;

LiquidCrystal_I2C lcd(0x27,LCD_LINE_LEN,2);   // 20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


//-------------------------------------
// temporary time database
//-------------------------------------

#define HOURS_REP_1      (1000 * 60 * 60)
#define HOURS_REP_24     (1000 * 60 * 60 * 24)

#define MAX_TIMES  100
    // temporary define for the circular buffer of millis() based pump-on times
    // to be changed to a database of time_t's

static uint32_t run_times[MAX_TIMES];
static int run_head = 0;

void addRun()
{
    run_times[run_head++] = millis();
    if (run_head >= MAX_TIMES) run_head = 0;
}

int countRuns(uint32_t hours_rep)
{
    int count = 0;
    int head = run_head;
    uint32_t now = millis();
    uint32_t cutoff = hours_rep >= now ? 1 : now-hours_rep;
    for (int i=0; i<MAX_TIMES; i++)
    {
        uint32_t time = run_times[head];
        if (head >= MAX_TIMES) head = 0;
        if (time && time > cutoff) count++;
    }
    return count;
}


//--------------------------------
// implementation
//--------------------------------

bilgeAlarm::bilgeAlarm()
{
    bilge_alarm = this;
    addValues(m_bilge_values,NUM_BILGE_VALUES);
    setTabLayouts(dash_items,device_items);
}



void bilgeAlarm::setup()
{
    LOGD("bilgeAlarm::setup(%s) started",getVersion());
    proc_entry();

    pinMode(PIN_ALARM,OUTPUT);
    pinMode(PIN_RELAY,OUTPUT);
    pinMode(PIN_ONBOARD_LED,OUTPUT);
    pinMode(PIN_OTHER_LED,OUTPUT);

    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_RELAY,0);
    digitalWrite(PIN_ONBOARD_LED,0);
    digitalWrite(PIN_OTHER_LED,0);

    pinMode(PIN_BUTTON1,  INPUT_PULLDOWN);
    pinMode(PIN_BUTTON2,  INPUT_PULLDOWN);
    pinMode(PIN_BUTTON3,  INPUT_PULLDOWN);
    pinMode(PIN_BUTTON4,  INPUT_PULLDOWN);
    pinMode(PIN_PUMP1_ON, INPUT_PULLDOWN);
    pinMode(PIN_PUMP2_ON, INPUT_PULLDOWN);
    // pinMode(PIN_5V_IN,    INPUT);
    // pinMode(PIN_12V_IN,   INPUT);

    lcd.init();                      // initialize the lcd
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("bilgeAlarm");
    lcd.setCursor(0,1);
    lcd.print(getVersion());

    myIOTDevice::setup();

    LOGI("starting alarmTask");
    xTaskCreate(alarmTask,
        "alarmTask",
        DEBUG_ALARM ? 4096 : 1024,  // very small stack unless debuggin
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL);

    proc_leave();
    LOGD("bilgeAlarm::setup(%s) completed",getVersion());
}


void bilgeAlarm::suppressError()
{
    LOGD("suppressError()");
    setAlarmState(_alarm_state | ALARM_STATE_SUPPRESSED);
}


void bilgeAlarm::clearError()
{
    LOGD("clearError()");
    setAlarmState(0);
    setState(_state & ~(
        STATE_EMERGENCY |
        STATE_TOO_OFTEN_HOUR |
        STATE_TOO_OFTEN_DAY |
        STATE_TOO_LONG |
        STATE_CRITICAL_TOO_LONG ));
}


#if DEBUG_STATES
    static String stateString(uint32_t state, enumValue *ptr)
    {
        int mask = 1;
        String rslt = "";
        while (*ptr)
        {
            if (state & mask)
            {
                if (rslt != "") rslt += ",";
                rslt += *ptr;
            }
            mask <<= 1;
            ptr++;
        }
        return rslt;
    }

    static void debugState(const char *what, bool *bptr, uint32_t in, uint32_t rslt, enumValue *ptr)
    {
        String bval = "";
        if (bptr)
        {
            bval = *bptr;
            bval += ",";
        }
        LOGD("%s(%s0x%08x:%s)  ==> 0x%08x:%s",
            what,
            bval,
            in,
            stateString(in,ptr).c_str(),
            rslt,
            stateString(rslt,ptr).c_str());
    }
#else
    #define debugState(a,b,c,d,e)
#endif



void bilgeAlarm::setState(uint32_t state)
{
    bilge_alarm->setBenum(ID_STATE,state);
    debugState("setState",NULL,state,_state,systemStates);
}


void bilgeAlarm::setAlarmState(uint32_t state)
{
    bilge_alarm->setBenum(ID_ALARM_STATE,state);
    debugState("setAlarmState",NULL,state,_alarm_state,alarmStates);
}


//------------------------------------
// alarmTask
//------------------------------------

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


void bilgeAlarm::alarmTask(void *param)
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
        if (last_alarm_state != _alarm_state)
        {
            #if DEBUG_ALARMS
                LOGD("alarm_state_changed to 0x%02x",_alarm_state);
            #endif

            last_alarm_state = _alarm_state;

            alarm_time = 0;
            alarm_on =
                (_alarm_state & ALARM_STATE_ANY) &&
                !(_alarm_state & ALARM_STATE_SUPPRESSED);

            if (!alarm_on)
                digitalWrite(PIN_ALARM,0);
            else if (_alarm_state & ALARM_STATE_EMERGENCY)
                digitalWrite(PIN_ALARM,alarm_on);
        }
        else if (alarm_on &&
            _alarm_state < ALARM_STATE_EMERGENCY &&
            now > alarm_time + TIME_BETWEEN_ALARMS)
        {
            alarm_time = now;
            if (_alarm_state & ALARM_STATE_CRITICAL)
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


//---------------------------------------------------
// buttons and other hardware
//---------------------------------------------------

void bilgeAlarm::onLed(const myIOTValue *value, bool val)
{
    String id = value->getId();
    if (id == ID_ONBOARD_LED)
    {
        digitalWrite(PIN_ONBOARD_LED,val);
    }
    else if (id == ID_OTHER_LED)
    {
        digitalWrite(PIN_OTHER_LED,val);
    }
}


void bilgeAlarm::onLcdLine(const myIOTValue *value, const char *val)
{
    int line = value->getId() == ID_LCD_LINE1 ? 0 : 1;
    LOGD("onLCDLine(%d,%s)",line,val);

    char buf[LCD_LINE_LEN+1];
    int len = strlen(val);
    if (len > LCD_LINE_LEN) len = LCD_LINE_LEN;
    strncpy(buf,val,len);
    for (int i=len; i<LCD_LINE_LEN; i++)
        buf[i] = ' ';
    buf[LCD_LINE_LEN] = 0;

    lcd.setCursor(0,line);
    lcd.print(buf);
}


void bilgeAlarm::onDisabled(const myIOTValue *value, bool val)
{
    LOGD("onDisabled(%d)",val);
    if (val)
    {
        setState(0);
        setAlarmState(0);
        m_relay_time = 0;
        m_relay_delay_time = 0;
        digitalWrite(PIN_RELAY,0);
        bilge_alarm->setBool(ID_FORCE_RELAY,0);
    }
}



void bilgeAlarm::handleButtons()
{
    bool button1 = digitalRead(PIN_BUTTON1);
    bool button2 = digitalRead(PIN_BUTTON2);
    bool button3 = digitalRead(PIN_BUTTON3);
    bool button4 = digitalRead(PIN_BUTTON4);
    if (_alarm_state && !(_alarm_state & ALARM_STATE_SUPPRESSED))
    {
        if (button1 || button2 || button3 || button4)
            _alarm_state |= ALARM_STATE_SUPPRESSED;
    }
}



//-------------------------------------------------------
// STATE MACHINE (handleSensors() and onForceRelay()
//-------------------------------------------------------


void bilgeAlarm::onForceRelay(const myIOTValue *value, bool val)
{
    LOGD("onForceRelay(%d)",val);
    uint32_t new_state = _state;

    m_relay_time = 0;
    m_relay_delay_time = 0;

    if (val)
    {
        new_state &= ~(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA);
        new_state |= STATE_RELAY_FORCED | STATE_RELAY_ON;
    }
    else
    {
        m_suppress_next_after = 1;
        new_state &= ~(STATE_RELAY_FORCED | STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
    }

    digitalWrite(PIN_RELAY,val);
    if (new_state != _state)
        setState(new_state);
}



void bilgeAlarm::handleSensors()
    // There is an issue that we don't want to publish to WS or MQTT in the middle of time,
    // but I am going to ignore that for now and may eventually implement an asynchronous
    // publishing task in myIOTValues.cpp
{
    uint32_t now = millis();
    uint32_t new_state = _state;
    uint32_t new_alarm_state = _alarm_state;

    bool pump1_on = digitalRead(PIN_PUMP1_ON);
    bool pump2_on = digitalRead(PIN_PUMP2_ON);
    bool was_on1 = new_state & STATE_PUMP1_ON;
    bool was_on2 = new_state & STATE_PUMP2_ON;
    bool forced_on = new_state & STATE_RELAY_FORCED;
    bool emergency_on = new_state & STATE_RELAY_EMERGENCY;

    //--------------------------------------
    // PUMP2
    //--------------------------------------
    // pump2 is higher priority - it may turn on the emergency_relay
    // so should be detected first

    if (was_on2 != pump2_on && now > m_pump2_debounce_time)
    {
        LOGD("m_pump2_on=%d",pump2_on);
        m_pump2_debounce_time = now  + _pump_debounce;

        if (pump2_on)
        {
            new_state |= STATE_PUMP2_ON;
            if (!_disabled)
            {
                new_state |= STATE_EMERGENCY;
                new_alarm_state |= ALARM_STATE_CRITICAL | ALARM_STATE_EMERGENCY;
            }
        }
        else
        {
            new_state &= ~STATE_PUMP2_ON;
            if (!_disabled)
                new_alarm_state &= ~ALARM_STATE_EMERGENCY;
        }

        if (!_disabled && !forced_on && pump2_on &&_run_emergency)
        {
            new_state &= ~STATE_RELAY_EXTRA;
            new_state |= STATE_RELAY_EMERGENCY;
            emergency_on = 1;
        }
    }

    // extend the emergency relay as long as the pump2 switch is on

    if (pump2_on & emergency_on)
    {
        m_relay_time = now + _run_emergency * 1000;
    }


    //-----------------------------
    // PUMP1
    //-----------------------------

    if (was_on1 != pump1_on && now > m_pump1_debounce_time)
    {
        LOGD("m_pump1_on=%d",pump1_on);
        m_pump1_debounce_time = now + _pump_debounce;

        if (pump1_on)
        {
            new_state |= STATE_PUMP1_ON;
            m_start_duration = now;
        }
        else
        {
            new_state &= ~STATE_PUMP1_ON;
            m_start_duration = 0;
        }

        if (!_disabled && !forced_on && !emergency_on && _extra_run_time)
        {
            if (pump1_on && !_extra_run_mode)
            {
                new_state |= STATE_RELAY_EXTRA;
            }
            else if (!pump1_on && _extra_run_mode && !m_suppress_next_after)
            {
                new_state |= STATE_RELAY_EXTRA;
                m_relay_delay_time = now + _extra_run_delay;
                    // relay will come on after the switch has been off for EXTRA_RUN_DELAY millis
            }
        }

        m_suppress_next_after = 0;
    }

    // duration based alarms

    if (pump1_on && m_start_duration)
    {
        uint32_t duration = (now - m_start_duration + 999) / 1000;

        if (_err_run_time && duration > _err_run_time)
        {
            new_state |= STATE_TOO_LONG;
            new_alarm_state |= ALARM_STATE_ERROR;
        }
        if (_crit_run_time && duration > _crit_run_time)
        {
            new_state |= STATE_CRITICAL_TOO_LONG;
            new_alarm_state |= ALARM_STATE_CRITICAL;
        }
    }


    //------------------------------------
    // THE RELAY
    //------------------------------------

    if (!_disabled && !forced_on)
    {
        static uint32_t last_relay_state = 0;
        uint32_t relay_state = new_state & (
            STATE_RELAY_EMERGENCY |
            STATE_RELAY_EXTRA);
        bool extra_on = new_state & STATE_RELAY_EXTRA;
        bool emergency_on = new_state & STATE_RELAY_EMERGENCY;

        if (last_relay_state != relay_state &&
            now > m_relay_delay_time)
        {
            last_relay_state = relay_state;

            m_relay_time = 0;

            bool is_on = new_state & STATE_RELAY_ON;
            bool should_be_on = relay_state;
            if (is_on != should_be_on)
            {
                if (should_be_on)
                    new_state |= STATE_RELAY_ON;
                else
                {
                    new_state &= ~STATE_RELAY_ON;
                }
                LOGD("relay=%d",should_be_on);
                digitalWrite(PIN_RELAY,should_be_on);
            }

            // turning on a changed relay state

            if (should_be_on)
            {
                if (emergency_on && _run_emergency)
                    m_relay_time = now + _run_emergency * 1000;
                if (extra_on &&_extra_run_time)
                    m_relay_time = now + _extra_run_time * 1000;
            }
            else // turning off, clear relay_time, set specific relay debounce time for pump1 switch
            {
                m_pump1_debounce_time = now + _relay_debounce;
                m_suppress_next_after = 1;
            }
        }

        // if the state did not change, check for timer expiration

        else if (m_relay_time && now > m_relay_time)
        {
            LOGD("relay off");
            m_relay_time = 0;
            m_suppress_next_after = 1;
            new_state &= ~(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
            digitalWrite(PIN_RELAY,0);
        }
    }

    if (new_state != _state)
        setState(new_state);
    if (new_alarm_state != _alarm_state)
        setAlarmState(new_alarm_state);

}   // handleSensors()



//-----------------------------------
// loop()
//-----------------------------------

void bilgeAlarm::loop()
{
    myIOTDevice::loop();

    uint32_t now = millis();
    static uint32_t check_buttons = 0;
    static uint32_t check_sensors = 0;
    if (now > check_buttons + BUTTON_CHECK_TIME)
    {
        check_buttons = now;
        handleButtons();
    }
    if (now > check_sensors + _sense_millis)
    {
        check_sensors = now;
        handleSensors();
    }
}
