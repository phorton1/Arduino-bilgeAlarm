//-----------------------------------
// bilgeAlarm.cpp
//-----------------------------------
#include "bilgeAlarm.h"
#include "uiScreen.h"
#include "baExterns.h"
#include <myIOTLog.h>


#define DEBUG_STATES  1


#define BUTTON_CHECK_TIME  30
#define UI_UPDATE_TIME     30


//--------------------------------
// bilgeAlarm definition
//--------------------------------

#define DEFAULT_DISABLED            0          // enabled,disabled
#define DEFAULT_BACKLIGHT_SECS      0          // off,secs
#define DEFAULT_MENU_SECS            15

#define DEFAULT_ERR_RUN_TIME        10         // off,secs
#define DEFAULT_CRIT_RUN_TIME       30         // off,secs
#define DEFAULT_ERR_PER_HOUR        4          // off,num
#define DEFAULT_ERR_PER_DAY         20         // off,secs

#define DEFAULT_EXTRA_RUN_TIME      5          // off,secs
#define DEFAULT_EXTRA_RUN_MODE      1          // at_start, after_end
#define DEFAULT_EXTRA_RUN_DELAY     1000       // millis after pump goes off to start after_end

#define DEFAULT_SENSE_MILLIS        10         // millis
#define DEFAULT_PUMP_DEBOUNCE       500        // millis
#define DEFAULT_RELAY_DEBOUNCE      2000       // millis

#define DEFAULT_RUN_EMERGENCY       30         // off,secs


// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
    ID_STATE,
    ID_ALARM_STATE,
    ID_NUM_LAST_HOUR,
    ID_NUM_LAST_DAY,
    ID_NUM_LAST_WEEK,
    ID_HISTORY_LINK,
    ID_SINCE_LAST_RUN,
    ID_TIME_LAST_RUN,
    ID_DUR_LAST_RUN,
    ID_SUPPRESS,
    ID_CLEAR_ERROR,
    ID_CLEAR_HISTORY,
    ID_DISABLED,
#if HAS_LCD_LINE_VALUES
    ID_LCD_LINE1,
    ID_LCD_LINE2,
#endif
#ifdef WITH_POWER
    ID_DEVICE_VOLTS,
    ID_DEVICE_AMPS,
#endif
    ID_FORCE_RELAY,
#if TEST_VERSION
    ID_ONBOARD_LED,
    ID_OTHER_LED,
    ID_DEMO_MODE,
#endif
    ID_REBOOT,
    0
};

// what shows up on the "device" UI tab

static valueIdType config_items[] = {
#if TEST_VERSION
    ID_DEMO_MODE,
#endif
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
    ID_MENU_SECS,
    0
};


// enum strings

static enumValue disabledStates[] = {
    "enabled",
    "disabled",
    0};
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
static enumValue pumpExtraType[] = {
    "from_start",
    "after_end",
    0};


// value descriptors for bilgeAlaram

const valDescriptor bilgeAlarm::m_bilge_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,   VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,                     NULL,   BILGE_ALARM },        // override base class element

    { ID_SUPPRESS,         VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                     (void *) suppressAlarm },
    { ID_CLEAR_ERROR,      VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                     (void *) clearError },
    { ID_CLEAR_HISTORY,    VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                     (void *) clearHistory },

    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_state,         NULL,   { .enum_range = { 0, systemStates }} },
    { ID_ALARM_STATE,      VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_alarm_state,   NULL,   { .enum_range = { 0, alarmStates }} },
    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_state,         NULL,   { .enum_range = { 0, systemStates }} },
    { ID_TIME_LAST_RUN,    VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_time_last_run, },
    { ID_SINCE_LAST_RUN,   VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_HIST_TIME,  (void *) &_since_last_run,NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1, DEVICE_MAX_INT}}  },
    { ID_DUR_LAST_RUN,     VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_dur_last_run,  NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_HOUR,    VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_num_last_hour, NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_DAY,     VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_num_last_day,  NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_WEEK,    VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_num_last_week, NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },

    { ID_DISABLED,         VALUE_TYPE_ENUM,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_disabled,       (void *) onDisabled,  { .enum_range = { 0, disabledStates }} },
    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_backlight_secs, NULL,  { .int_range = { DEFAULT_BACKLIGHT_SECS,   30,  3600}}  },
    { ID_MENU_SECS,        VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,   (void *) &_menu_secs, NULL,  { .int_range = {DEFAULT_MENU_SECS,   0,  3600}}  },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_run_time,   NULL,  { .int_range = { DEFAULT_ERR_RUN_TIME,      0,  3600}}  },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_crit_run_time,  NULL,  { .int_range = { DEFAULT_CRIT_RUN_TIME,     0,  3600}}  },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_per_hour,   NULL,  { .int_range = { DEFAULT_ERR_PER_HOUR,      0,  MAX_RUN_HISTORY}}   },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_per_day,    NULL,  { .int_range = { DEFAULT_ERR_PER_DAY,       0,  MAX_RUN_HISTORY}}   },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_run_emergency,  NULL,  { .int_range = { DEFAULT_RUN_EMERGENCY,     0,  3600}}  },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_extra_run_time, NULL,  { .int_range = { DEFAULT_EXTRA_RUN_TIME,    0,  3600}}  },
    { ID_EXTRA_RUN_MODE,   VALUE_TYPE_ENUM,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_mode, NULL,  { .enum_range = { 0, pumpExtraType }} },
    { ID_EXTRA_RUN_DELAY,  VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_delay,NULL,  { .int_range = { DEFAULT_EXTRA_RUN_DELAY,   5,  30000}} },
    { ID_SENSE_MILLIS,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_sense_millis,   NULL,  { .int_range = { DEFAULT_SENSE_MILLIS,      5,  30000}} },
    { ID_PUMP_DEBOUNCE,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pump_debounce,  NULL,  { .int_range = { DEFAULT_PUMP_DEBOUNCE,     5,  30000}} },
    { ID_RELAY_DEBOUNCE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_relay_debounce, NULL,  { .int_range = { DEFAULT_RELAY_DEBOUNCE,    5,  30000}} },

    { ID_FORCE_RELAY,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_FORCE_RELAY,    (void *) onForceRelay },

#if HAS_LCD_LINE_VALUES
    { ID_LCD_LINE1,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line1,      (void *) onLcdLine },
    { ID_LCD_LINE2,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line2,      (void *) onLcdLine },
#endif
#if TEST_VERSION
    { ID_ONBOARD_LED,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_ONBOARD_LED,    (void *) onLed, },
    { ID_OTHER_LED,        VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_OTHER_LED,      (void *) onLed, },
    { ID_DEMO_MODE,        VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_DEMO_MODE,      },
#endif

    // following places a custom link on the dashboard page

    { ID_HISTORY_LINK,   VALUE_TYPE_STRING,     VALUE_STORE_PUB,       VALUE_STYLE_READONLY,   (void *) &_history_link,     },

};


#define NUM_BILGE_VALUES (sizeof(m_bilge_values)/sizeof(valDescriptor))

// values

uint32_t bilgeAlarm::_state;
uint32_t bilgeAlarm::_alarm_state;
time_t   bilgeAlarm::_time_last_run;
int      bilgeAlarm::_since_last_run;
int      bilgeAlarm::_dur_last_run;
int      bilgeAlarm::_num_last_hour;
int      bilgeAlarm::_num_last_day;
int      bilgeAlarm::_num_last_week;
String   bilgeAlarm::_history_link;


bool bilgeAlarm::_disabled;
int  bilgeAlarm::_backlight_secs;
int  bilgeAlarm::_menu_secs;
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

#if HAS_LCD_LINE_VALUES
    String bilgeAlarm::_lcd_line1;
    String bilgeAlarm::_lcd_line2;
#endif
#if TEST_VERSION
    bool bilgeAlarm::_ONBOARD_LED;
    bool bilgeAlarm::_OTHER_LED;
    bool bilgeAlarm::_DEMO_MODE;
#endif

// working vars

uint32_t bilgeAlarm::m_pump1_debounce_time;
uint32_t bilgeAlarm::m_pump2_debounce_time;
uint32_t bilgeAlarm::m_relay_delay_time;
uint32_t bilgeAlarm::m_relay_time;
bool     bilgeAlarm::m_suppress_next_after;

bilgeAlarmState_t bilgeAlarm::m_publish_state;

bilgeAlarm *bilge_alarm = NULL;
// iotLCD_UI *my_ui = NULL;

static bool count_this_run = 0;


//--------------------------------
// implementation
//--------------------------------

bilgeAlarm::bilgeAlarm()
{
    bilge_alarm = this;
    addValues(m_bilge_values,NUM_BILGE_VALUES);
    setTabLayouts(dash_items,config_items);
}



void bilgeAlarm::setup()
{
    LOGD("bilgeAlarm::setup(%s) started",getVersion());
    proc_entry();

    pinMode(PIN_ALARM,OUTPUT);
    pinMode(PIN_RELAY,OUTPUT);
    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_RELAY,0);

#if TEST_VERSION
    pinMode(PIN_ONBOARD_LED,OUTPUT);
    pinMode(PIN_OTHER_LED,OUTPUT);
    digitalWrite(PIN_ONBOARD_LED,0);
    digitalWrite(PIN_OTHER_LED,0);
#endif

    pinMode(PIN_PUMP1_ON, INPUT_PULLDOWN);
    pinMode(PIN_PUMP2_ON, INPUT_PULLDOWN);
    // pinMode(PIN_5V_IN,    INPUT);
    // pinMode(PIN_12V_IN,   INPUT);

    int button_pins[] = {PIN_BUTTON0,PIN_BUTTON1,PIN_BUTTON2};
    // ui_screen =
    new uiScreen(button_pins);

    myIOTDevice::setup();

    startAlarm();

    _history_link = "<a href='/custom/getHistory?uuid=";
    _history_link += getUUID();
    _history_link += "' target='_blank'>History</a>";

    LOGI("starting stateTask");
    xTaskCreatePinnedToCore(stateTask,
        "stateTask",
        4096,           // task stack
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL,           // returned task handle
        ESP32_CORE_ARDUINO);

    proc_leave();
    LOGD("bilgeAlarm::setup(%s) completed",getVersion());
}



//------------------------------------
// state setters
//------------------------------------

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
        return rslt == "" ? "NONE" : rslt;
    }

    static void debugState(const char *what, uint32_t in, uint32_t rslt, enumValue *ptr)
    {
        LOGD("%s(%s) => %s",
            what,
            stateString(in,ptr).c_str(),
            stateString(rslt,ptr).c_str());
    }
#else
    #define debugState(a,b,c,d,e)
#endif


void bilgeAlarm::setState(uint32_t state)
{
    _state = state;
    debugState("setState",state,_state,systemStates);
}
void bilgeAlarm::addState(uint32_t state)
{
    _state |= state;
    debugState("addState",state,_state,systemStates);
}
void bilgeAlarm::clearState(uint32_t state)
{
    _state &= ~state;
    debugState("clearState",state,_state,systemStates);
}

void bilgeAlarm::setAlarmState(uint32_t alarm_state)
{
    _alarm_state = alarm_state;
    debugState("setAlarmState",alarm_state,_alarm_state,alarmStates);
}
void bilgeAlarm::addAlarmState(uint32_t alarm_state)
{
    _alarm_state |= alarm_state;
    debugState("addAlarmState",alarm_state,_alarm_state,alarmStates);
}
void bilgeAlarm::clearAlarmState(uint32_t alarm_state)
{
    _alarm_state &= ~alarm_state;
    debugState("clearAlarmState",alarm_state,_alarm_state,alarmStates);
}



//---------------------------------------
// commands
//---------------------------------------
// called from UI and myIOTValue.cpp invoke() via registered fxn


void bilgeAlarm::suppressAlarm()
{
    LOGU("suppressAlarm()");
    addAlarmState(ALARM_STATE_SUPPRESSED);
}

void bilgeAlarm::clearHistory()
{
    LOGU("clearHistory()");
    clearError();
    clearRuns();
    _time_last_run = 0;
    _since_last_run = 0;
    _dur_last_run = 0;
}


void bilgeAlarm::clearError()
{
    LOGU("clearError()");
    if (_alarm_state)
    {
        digitalWrite(PIN_RELAY,0);

        if (count_this_run)
        {
            count_this_run = 0;
            endRun();
        }
        // when we clear either of these errors, we set the time window
        // so that the count goes down to ONE UNDER the error limit,
        // to prevent false retriggers, but allow the next pump run
        // to re-trigger the error.  Changing the error parameters,
        // clearing the history, and disabling the alarm are other
        // user strategies to cope with pesky count alarms.
        //
        // hopefully the count will roll off before the pump goes back on

        if (_state & STATE_TOO_OFTEN_HOUR)
            setRunWindow(COUNT_HOUR,_err_per_hour - 1);
        if (_state & STATE_TOO_OFTEN_DAY)
            setRunWindow(COUNT_DAY,_err_per_day - 1);

        // we manually force the relay off in case it is on


        setState(0);
        setAlarmState(0);

        m_pump1_debounce_time = millis() + _relay_debounce;
        m_pump2_debounce_time = millis() + _relay_debounce;

    }
}


//---------------------------------------------------
// "on" handlers
//---------------------------------------------------

#if HAS_LCD_LINE_VALUES
    void bilgeAlarm::onLcdLine(const myIOTValue *value, const char *val)
    {
        int line = value->getId() == ID_LCD_LINE1 ? 0 : 1;
        LOGD("onLCDLine(%d,%s)",line,val);
    }
#endif
#if TEST_VERSION
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
#endif



void bilgeAlarm::onValueChanged(const myIOTValue *value, valueStore from)
    // called from myIOTValue.cpp::publish();
{
    ui_screen->onValueChanged(value,from);
}


String bilgeAlarm::onCustomLink(const String &path)
    // called from myIOTHTTP.cpp::handleRequest()
{
    if (path.startsWith("getHistory"))
    {
        return historyHTML();
    }
    return "";
}


void bilgeAlarm::onDisabled(const myIOTValue *value, bool val)
    // called from myIOTValue.cpp setBool() via registered fxn
{
    LOGU("onDisabled(%d)",val);
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


void bilgeAlarm::onForceRelay(const myIOTValue *value, bool val)
    // called from myIOTValue.cpp setBool() via registered fxn
{
    LOGD("onForceRelay(%d)",val);
    uint32_t new_state = _state;

    m_relay_time = 0;
    m_relay_delay_time = 0;

    if (val)
    {
        LOGU("FORCE RELAY ON");
        clearState(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA);
        addState(STATE_RELAY_FORCED | STATE_RELAY_ON);
    }
    else
    {
        LOGU("FORCE RELAY OFF");
        m_suppress_next_after = 1;
        clearState(STATE_RELAY_FORCED | STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
    }

    digitalWrite(PIN_RELAY,val);
    m_pump1_debounce_time = millis() + _relay_debounce;
}



//-------------------------------------------------------
// THE STATE MACHINE - stateMachine()
//-------------------------------------------------------
// The state machine is time critical and runs as a task.
// It cannot use the set() methods inline.
// Therefore we keep a 'broadcast state' and publish that in
// loop() when it changes.
//
// This is very complicated.
// We don't count the runs until the switch goes off, but
// we want to add the TOO_MANY_DAY and HOUR flags to the
// history, so we have to keep track of the PREVIOUS count.


void bilgeAlarm::stateTask(void *param)
{
    delay(1200);
    LOGI("starting stateTask loop on core(%d)",xPortGetCoreID());
    delay(1200);
    while (1)
    {
        vTaskDelay(bilge_alarm->_sense_millis / portTICK_PERIOD_MS);
        bilge_alarm->stateMachine();
    }
}


void bilgeAlarm::stateMachine()
    // THIS IS THE STATE MACHINE FOR THE BILGE ALARM
    // There is an issue that we don't want to publish to WS or MQTT in the middle of time,
    // but I am going to ignore that for now and may eventually implement an asynchronous
    // publishing task in myIOTValues.cpp
{
    uint32_t now = millis();
    time_t time_now = time(NULL);
    uint32_t new_state = _state;
    uint32_t _alarm_state = _alarm_state;

    bool pump1_on = digitalRead(PIN_PUMP1_ON);
    bool pump2_on = digitalRead(PIN_PUMP2_ON);
    bool was_on1 = _state & STATE_PUMP1_ON;
    bool was_on2 = _state & STATE_PUMP2_ON;
    bool pump1_valid = now > m_pump1_debounce_time;
    bool pump2_valid = now > m_pump2_debounce_time;
    bool forced_on = _state & STATE_RELAY_FORCED;
    bool emergency_on = _state & STATE_RELAY_EMERGENCY;
    bool after_extra = _extra_run_mode && _state & STATE_RELAY_EXTRA;

    static uint32_t subtract_relay;


    // start the run

    if (!count_this_run)
    {
        if ((pump2_on && pump2_valid) ||
            (pump1_on && pump1_valid && !forced_on && !after_extra))
        {
            subtract_relay = 0;
            startRun();
            _time_last_run = time_now;
            _since_last_run = (int32_t) time_now;
            _dur_last_run = 0;
            count_this_run = 1;
        }
    }

    //--------------------------------------
    // PUMP2
    //--------------------------------------
    // pump2 is higher priority - it may turn on the emergency_relay
    // so should be detected first

    if (pump2_valid && was_on2 != pump2_on)
    {
        LOGI("m_pump2_on=%d",pump2_on);
        m_pump2_debounce_time = now  + _pump_debounce;

        if (pump2_on)
        {
            addState(STATE_PUMP2_ON);
            setRunFlags(STATE_EMERGENCY);

            if (!_disabled)
            {
                addAlarmState(ALARM_STATE_EMERGENCY);
                if (!(_state & STATE_EMERGENCY))
                {
                    addState(STATE_EMERGENCY);
                    LOGU("ALARM - EMERGENCY PUMP ON!!");
                }
                if (_run_emergency)
                {
                    LOGU("EMERGENCY RELAY ON");
                    clearState(STATE_RELAY_EXTRA | STATE_RELAY_FORCED);
                    addState(STATE_RELAY_EMERGENCY);
                    emergency_on = 1;
                }
            }
        }
        else
        {
            clearState(STATE_PUMP2_ON);
            if (!_disabled)
            {
                if (_alarm_state & ALARM_STATE_EMERGENCY)
                {
                    LOGU("ALARM - EMERGENCY PUMP OFF - DOWNGRADE TO CRITICAL");
                    clearState(ALARM_STATE_EMERGENCY);
                    addState(ALARM_STATE_CRITICAL);
                }
            }
        }
    }

    // extend the emergency relay as long as the pump2 switch is on

    if (pump2_valid && pump2_on && emergency_on)
    {
        m_relay_time = now + _run_emergency * 1000;
    }


    //-----------------------------
    // PUMP1
    //-----------------------------

    if (pump1_valid && was_on1 != pump1_on)
    {
        LOGI("m_pump1_on=%d",pump1_on);
        m_pump1_debounce_time = now + _pump_debounce;

        if (pump1_on)
        {
            addState(STATE_PUMP1_ON);
        }
        else
        {
            clearState(STATE_PUMP1_ON);
        }

        if (!_disabled && !forced_on && !emergency_on && _extra_run_time)
        {
            if (pump1_on && !_extra_run_mode)
            {
                LOGU("EXTRA RELAY ON");
                addState(STATE_RELAY_EXTRA);
            }
            else if (!pump1_on && _extra_run_mode && !m_suppress_next_after)
            {
                addState(STATE_RELAY_EXTRA);
                m_relay_delay_time = now + _extra_run_delay;
                    // relay will come on after the switch has been off for EXTRA_RUN_DELAY millis
            }
        }

        m_suppress_next_after = 0;
    }


    //---------------------------
    // duration based alarms
    //---------------------------

    if (count_this_run && pump1_valid && pump1_on)
    {
        // the history reports the actual duration, including the relay on times,
        // but we only want to give errors based on the time the relay was off.
        // so we keep a static of any relay that turns on between startRun()
        // and endRun() and subtract it for this calculation

        // calculate duration that the pump has been on by itself

        int duration = time_now - getStartDuration();
        duration -= subtract_relay;

        // raise duration based alarms

        if (_err_run_time && duration > _err_run_time)
        {
            setRunFlags(STATE_TOO_LONG);
            if (!_disabled && !(_state & STATE_TOO_LONG))
            {
                LOGU("ALARM - PUMP TOO LONG");
                addState(STATE_TOO_LONG);
                addAlarmState(ALARM_STATE_ERROR);
            }
        }

        if (_crit_run_time && duration > _crit_run_time)
        {
            setRunFlags(STATE_CRITICAL_TOO_LONG);
            if (!_disabled && !(_state & STATE_CRITICAL_TOO_LONG))
            {
                LOGU("ALARM - PUMP CRITICAL TOO LONG");
                addState(STATE_CRITICAL_TOO_LONG);
                addAlarmState(ALARM_STATE_CRITICAL);
            }
        }
    }

    //------------------------------------
    // THE RELAY
    //------------------------------------

    if (!_disabled && !forced_on)
    {
        static uint32_t last_relay_state = 0;
        uint32_t relay_state = _state & (
            STATE_RELAY_EMERGENCY |
            STATE_RELAY_EXTRA);
        bool extra_on = _state & STATE_RELAY_EXTRA;
        bool emergency_on = _state & STATE_RELAY_EMERGENCY;

        if (last_relay_state != relay_state &&
            now > m_relay_delay_time)
        {
            last_relay_state = relay_state;

            m_relay_time = 0;

            bool is_on = _state & STATE_RELAY_ON;
            bool should_be_on = relay_state;
            if (is_on != should_be_on)
            {
                // the LOGU's are such that we only show a message when
                // the kind of relay is turned on.

                if (should_be_on)
                {
                    addState(STATE_RELAY_ON);
                    if (m_relay_delay_time)
                        LOGU("DEFERRED EXTRA RELAY ON");
                    m_relay_delay_time = 0;
                }

                // I think this else is in case they disable the system
                // while there are relays on.

                else
                {
                    LOGU("DISABLED - RELAY_OFF");
                    clearState(STATE_RELAY_ON);
                }

                // just for good measure I also output a LOGI showing the state change

                LOGI("relay=%d",should_be_on);
                digitalWrite(PIN_RELAY,should_be_on);
            }

            // turning on a changed relay state

            if (should_be_on)
            {
                if (emergency_on && _run_emergency)
                {
                    subtract_relay = _run_emergency;
                    m_relay_time = now + _run_emergency * 1000;
                }
                if (extra_on &&_extra_run_time)
                {
                    if (!_extra_run_mode)
                        subtract_relay = _extra_run_time;
                    m_relay_time = now + _extra_run_time * 1000;
                }
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
            LOGU("AUTO RELAY_OFF");
            m_relay_time = 0;
            m_suppress_next_after = 1;
            clearState(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
            digitalWrite(PIN_RELAY,0);
            m_pump1_debounce_time = now + _relay_debounce;
        }
    }

    //---------------------------
    // count based alarms
    //---------------------------

    static time_t last_clock_time = 0;
    if (time_now != last_clock_time)
    {
        last_clock_time = time_now;

        int count_hour = countRuns(COUNT_HOUR);
        int count_day = countRuns(COUNT_DAY);

        // not to do with errors, but we use the timer to also update the weekly count
        // EVERY SECOND .. could get slow ...

        _num_last_week = countRuns(COUNT_WEEK);

        // set check_clear if there are only TOO_OFTEN ERRORS
        // to auto clear them below, and keep another boolean
        // to see if we should clear the alarm state as well

        #define ANY_OTHER_ALARM (ALARM_STATE_CRITICAL | ALARM_STATE_EMERGENCY)
        #define ANY_OTHER_ERROR (STATE_EMERGENCY | STATE_TOO_LONG | STATE_CRITICAL_TOO_LONG)

        bool only_error = !(_alarm_state & ANY_OTHER_ALARM);
        bool only_count = !(_state & ANY_OTHER_ERROR);
        bool check_clear = only_error && only_count;
        bool cleared_alarm = 0;

        #if 0
            if (count_hour != _num_last_hour ||
                count_day != _num_last_day)
            {
                LOGD("count_hour     = %-3d          old=%d         err=%d",count_hour,_num_last_hour,_err_per_hour);
                LOGD("count_hour     = %-3d          old=%d         err=%d",count_day, _num_last_day, _err_per_day);
                LOGD("check_clear    = %-3d   only_error=%d  only_count=%d",check_clear,only_error,only_count);
            }
        #endif

        if (count_hour != _num_last_hour)
        {
            _num_last_hour = count_hour;

            if (_err_per_hour && _num_last_hour >= _err_per_hour)
            {
                setRunFlags(STATE_TOO_OFTEN_HOUR);

                if (!_disabled)
                {
                    LOGU("ALARM - TOO MANY PER HOUR");
                    addState(STATE_TOO_OFTEN_HOUR);
                    addAlarmState(ALARM_STATE_ERROR);
                }
            }
            else if (check_clear &&
                     (_state & STATE_TOO_OFTEN_HOUR) &&
                     _num_last_hour < _err_per_hour)
            {
                LOGU("ALARM - SELF CLEARING TOO MANY PER HOUR");
                clearState(STATE_TOO_OFTEN_HOUR);
                cleared_alarm = 1;
            }
        }

        if (count_day != _num_last_day)
        {
            _num_last_day = count_day;

            if (_err_per_day && _num_last_day >= _err_per_day)
            {
                setRunFlags(STATE_TOO_OFTEN_DAY);

                if (!_disabled)
                {
                    LOGU("ALARM - TOO MANY PER DAY");
                    addState(STATE_TOO_OFTEN_DAY);
                    addAlarmState(ALARM_STATE_ERROR);
                }
            }
            else if (check_clear &&
                     (_state & STATE_TOO_OFTEN_DAY) &&
                     _num_last_day < _err_per_day)
            {
                LOGU("ALARM - SELF CLEARING TOO MANY PER DAY");
                clearState(STATE_TOO_OFTEN_DAY);
                cleared_alarm = 1;
            }
        }

        // if we cleared at least one, check if there's any count errors left
        // and if not clear the error state

        #define OFTEN_ERRORS (STATE_TOO_OFTEN_HOUR | STATE_TOO_OFTEN_DAY)

        if (cleared_alarm && !(_state & OFTEN_ERRORS))
        {
            LOGU("ALARM - SELF CLEARING ALARM_STATE");
            clearError();
        }

    }   // every second


    // end the run

    if (count_this_run && pump1_valid && pump2_valid && !pump1_on && !pump2_on)
    {
        _dur_last_run = endRun();
        count_this_run = 0;
        subtract_relay = 0;
    }

}   // stateMachine()



void bilgeAlarm::publishState()
{
    if (m_publish_state.state != _state)
    {
        m_publish_state.state = _state;
        setBenum(ID_STATE,m_publish_state.state);
    }
    if (m_publish_state.alarm_state != _alarm_state)
    {
        m_publish_state.alarm_state = _alarm_state;
        setBenum(ID_ALARM_STATE,m_publish_state.alarm_state);
    }
    if (m_publish_state.time_last_run != _time_last_run)
    {
        m_publish_state.time_last_run = _time_last_run;
        setTime(ID_TIME_LAST_RUN,m_publish_state.time_last_run);
    }
    if (m_publish_state.since_last_run != _since_last_run)
    {
        m_publish_state.since_last_run = _since_last_run;
        setInt(ID_SINCE_LAST_RUN,m_publish_state.since_last_run);
    }
    if (m_publish_state.dur_last_run != _dur_last_run)
    {
        m_publish_state.dur_last_run = _dur_last_run;
        setInt(ID_DUR_LAST_RUN,m_publish_state.dur_last_run);
    }
    if (m_publish_state.num_last_hour != _num_last_hour)
    {
        m_publish_state.num_last_hour = _num_last_hour;
        setInt(ID_NUM_LAST_HOUR,m_publish_state.num_last_hour);
    }
    if (m_publish_state.num_last_day != _num_last_day)
    {
        m_publish_state.num_last_day = _num_last_day;
        setInt(ID_NUM_LAST_DAY,m_publish_state.num_last_day);
    }
    if (m_publish_state.num_last_week != _num_last_week)
    {
        m_publish_state.num_last_week = _num_last_week;
        setInt(ID_NUM_LAST_WEEK,m_publish_state.num_last_week);
    }
}

//-----------------------------------
// loop()
//-----------------------------------

void bilgeAlarm::loop()
{
    myIOTDevice::loop();
    bilge_alarm->publishState();
    ui_screen->loop();
}
