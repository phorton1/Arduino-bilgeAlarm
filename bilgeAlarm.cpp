//-----------------------------------
// bilgeAlarm.cpp
//-----------------------------------
#include "bilgeAlarm.h"
#include "uiScreen.h"
#include "baAlarm.h"
#include "baHistory.h"
#include <myIOTLog.h>


#define DEBUG_STATES  1

#define DEBUG_SWITCHES 0
    // set this to merely output the switch values once per second



#define BUTTON_CHECK_TIME  30
#define UI_UPDATE_TIME     30


//--------------------------------
// bilgeAlarm definition
//--------------------------------

#define DEFAULT_DISABLED            0          // enabled,disabled
#define DEFAULT_BACKLIGHT_SECS      0          // off,secs
#define MIN_BACKLIGHT_SECS          30
#define DEFAULT_MENU_SECS           15         // off
#define MIN_MENU_SECS               15

#define DEFAULT_ERR_RUN_TIME        10         // off,secs
#define DEFAULT_CRIT_RUN_TIME       30         // off,secs
#define DEFAULT_ERR_PER_HOUR        4          // off,num
#define DEFAULT_ERR_PER_DAY         20         // off,secs

#define DEFAULT_EXTRA_RUN_TIME      5          // off,secs
#define DEFAULT_EXTRA_RUN_MODE      0          // at_start, after_end
#define DEFAULT_EXTRA_RUN_DELAY     1000       // millis after pump goes off to start after_end

#define DEFAULT_SENSE_MILLIS        10         // millis
#define DEFAULT_PUMP_DEBOUNCE       500        // millis
#define DEFAULT_RELAY_DEBOUNCE      2000       // millis

#define DEFAULT_RUN_EMERGENCY       30         // off,secs

#define DEFAULT_SW_THRESHOLD        900        // working value for analogRead() of pump switches
    // There was an issue that when the Alarm came on due to digitalRead,
    // it drew enough power (from the test harness) that the PUMP2_ON digital
    // read subsequently returned 0, thus causing the system to cycle through
    // pump ON/OFF every few seconds  This is likely excerbated by my voltage
    // divider being a little too protective.   Changing it to analogRead with
    // an agressive (and probably to-become parameterized threshold) seems to
    // have worked around the problem, although you can see the whole system
    // is being starved of power through the process.




// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
    ID_STATE,
    ID_ALARM_STATE,
    ID_NUM_LAST_HOUR,
    ID_NUM_LAST_DAY,
    ID_NUM_LAST_WEEK,
    ID_SUPPRESS,
    ID_CLEAR_ERROR,
    ID_CLEAR_HISTORY,
    ID_POWER_12V,
    ID_POWER_5V,
    ID_HISTORY_LINK,
    ID_SINCE_LAST_RUN,
    ID_TIME_LAST_RUN,
    ID_DUR_LAST_RUN,
    ID_DISABLED,
#if HAS_LCD_LINE_VALUES
    ID_LCD_LINE1,
    ID_LCD_LINE2,
#endif
    ID_FORCE_RELAY,
    ID_REBOOT,
    0
};

// what shows up on the "device" UI tab

static valueIdType config_items[] = {
    ID_SELFTEST,
    ID_DISABLED,
    ID_ERR_RUN_TIME,
    ID_CRIT_RUN_TIME,
    ID_ERR_PER_HOUR,
    ID_ERR_PER_DAY,
    ID_RUN_EMERGENCY,
    ID_EXTRA_RUN_TIME,
    ID_EXTRA_RUN_MODE,
    ID_EXTRA_RUN_DELAY,
    ID_LED_BRIGHT,
    ID_EXT_LED_BRIGHT,
    ID_BACKLIGHT_SECS,
    ID_MENU_SECS,
    ID_CALIB_12V,
    ID_CALIB_5V,
    ID_SENSE_MILLIS,
    ID_PUMP_DEBOUNCE,
    ID_RELAY_DEBOUNCE,
    ID_SW_THRESHOLD,
    0
};


// enum strings

static enumValue disabledStates[] = {
    "enabled",
    "silent",
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
    "WAY_TOO_LONG",
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

    { ID_SELFTEST,         VALUE_TYPE_COMMAND,  VALUE_STORE_SUB,      VALUE_STYLE_NONE,       NULL,                     (void *) selfTest },
    { ID_SUPPRESS,         VALUE_TYPE_COMMAND,  VALUE_STORE_SUB,      VALUE_STYLE_NONE,       NULL,                     (void *) suppressAlarm },
    { ID_CLEAR_ERROR,      VALUE_TYPE_COMMAND,  VALUE_STORE_SUB,      VALUE_STYLE_NONE,       NULL,                     (void *) clearError },
    { ID_CLEAR_HISTORY,    VALUE_TYPE_COMMAND,  VALUE_STORE_SUB,      VALUE_STYLE_VERIFY,     NULL,                     (void *) clearHistory },

    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.state,         NULL,   { .enum_range = { 0, systemStates }} },
    { ID_ALARM_STATE,      VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &m_publish_state.alarm_state,   NULL,   { .enum_range = { 0, alarmStates }} },
    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.state,         NULL,   { .enum_range = { 0, systemStates }} },
    { ID_TIME_LAST_RUN,    VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.time_last_run, },
    { ID_SINCE_LAST_RUN,   VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_HIST_TIME,  (void *) &m_publish_state.since_last_run,NULL,   { .int_range = { 0, DEVICE_MIN_INT, DEVICE_MAX_INT}}  },
    { ID_DUR_LAST_RUN,     VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.dur_last_run,  NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_HOUR,    VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.num_last_hour, NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_DAY,     VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.num_last_day,  NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },
    { ID_NUM_LAST_WEEK,    VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &m_publish_state.num_last_week, NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },

    { ID_DISABLED,         VALUE_TYPE_ENUM,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_disabled,       (void *) onDisabled,  { .enum_range = { 0, disabledStates }} },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_run_time,   NULL,  { .int_range = { DEFAULT_ERR_RUN_TIME,      0,  3600}}  },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_crit_run_time,  NULL,  { .int_range = { DEFAULT_CRIT_RUN_TIME,     0,  3600}}  },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_per_hour,   NULL,  { .int_range = { DEFAULT_ERR_PER_HOUR,      0,  MAX_RUN_HISTORY-1}}   },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_err_per_day,    NULL,  { .int_range = { DEFAULT_ERR_PER_DAY,       0,  MAX_RUN_HISTORY-1}}   },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_run_emergency,  NULL,  { .int_range = { DEFAULT_RUN_EMERGENCY,     0,  3600}}  },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_extra_run_time, NULL,  { .int_range = { DEFAULT_EXTRA_RUN_TIME,    0,  3600}}  },
    { ID_EXTRA_RUN_MODE,   VALUE_TYPE_ENUM,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_mode, NULL,  { .enum_range = { 0, pumpExtraType }} },
    { ID_EXTRA_RUN_DELAY,  VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_delay,NULL,  { .int_range = { DEFAULT_EXTRA_RUN_DELAY,   5,  30000}} },
    { ID_SENSE_MILLIS,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_sense_millis,   NULL,  { .int_range = { DEFAULT_SENSE_MILLIS,      5,  30000}} },
    { ID_PUMP_DEBOUNCE,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pump_debounce,  NULL,  { .int_range = { DEFAULT_PUMP_DEBOUNCE,     5,  30000}} },
    { ID_RELAY_DEBOUNCE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_relay_debounce, NULL,  { .int_range = { DEFAULT_RELAY_DEBOUNCE,    5,  30000}} },
    { ID_SW_THRESHOLD,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_sw_threshold,   NULL,  { .int_range = { DEFAULT_SW_THRESHOLD,      5,  4095}} },


    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_backlight_secs, NULL,  { .int_range = { DEFAULT_BACKLIGHT_SECS,    MIN_BACKLIGHT_SECS, 3600}}  },
    { ID_MENU_SECS,        VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_menu_secs,      NULL,  { .int_range = { DEFAULT_MENU_SECS,         MIN_MENU_SECS,      3600}}  },
    { ID_LED_BRIGHT,       VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   NULL,                      (void *) onLedBright, { .int_range = { DEFAULT_LED_BRIGHT, 0, 255}}  },
    { ID_EXT_LED_BRIGHT,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   NULL,                      (void *) onLedBright, { .int_range = { DEFAULT_LED_BRIGHT, 0, 255}}  },

    { ID_FORCE_RELAY,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &_FORCE_RELAY,    (void *) onForceRelay },

    { ID_POWER_12V,        VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_power_12v,       NULL,   { .float_range = { 0.00, -50.00, 50.00 }} },
    { ID_POWER_5V,         VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_power_5v,        NULL,   { .float_range = { 0.00, -300.00, 300.00 }} },
    { ID_CALIB_12V,        VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_calib_12v,       NULL,   { .int_range = { 1000,     500,  1500}} },
    { ID_CALIB_5V,         VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_calib_5v,        NULL,   { .int_range = { 1000,     500,  1500}} },

#if HAS_LCD_LINE_VALUES
    { ID_LCD_LINE1,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line1,      (void *) onLcdLine },
    { ID_LCD_LINE2,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_LONG,       (void *) &_lcd_line2,      (void *) onLcdLine },
#endif

    // following places a custom link on the dashboard page

    { ID_HISTORY_LINK,   VALUE_TYPE_STRING,     VALUE_STORE_PUB,       VALUE_STYLE_READONLY,   (void *) &_history_link,     },

};


#define NUM_BILGE_VALUES (sizeof(m_bilge_values)/sizeof(valDescriptor))

// values (private)

uint32_t bilgeAlarm::_state;
uint32_t bilgeAlarm::_alarm_state;
time_t   bilgeAlarm::_time_last_run;
int      bilgeAlarm::_since_last_run;
int      bilgeAlarm::_dur_last_run;
int      bilgeAlarm::_num_last_hour;
int      bilgeAlarm::_num_last_day;
int      bilgeAlarm::_num_last_week;
String   bilgeAlarm::_history_link;

// values (public)

uint32_t  bilgeAlarm::_disabled;
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
int  bilgeAlarm::_sw_threshold;

float bilgeAlarm::_power_12v;
float bilgeAlarm::_power_5v;
int   bilgeAlarm::_calib_12v;
int   bilgeAlarm::_calib_5v;

bool bilgeAlarm::_FORCE_RELAY;

#if HAS_LCD_LINE_VALUES
    String bilgeAlarm::_lcd_line1;
    String bilgeAlarm::_lcd_line2;
#endif


// working vars

uint32_t bilgeAlarm::m_pump1_debounce_time;
uint32_t bilgeAlarm::m_pump2_debounce_time;
uint32_t bilgeAlarm::m_relay_delay_time;
uint32_t bilgeAlarm::m_relay_time;
bool     bilgeAlarm::m_suppress_next_after;
bool     bilgeAlarm::m_in_self_test;


bilgeAlarmState_t bilgeAlarm::m_publish_state;

bilgeAlarm *bilge_alarm = NULL;
// iotLCD_UI *my_ui = NULL;

static bool count_this_run = 0;


#if WITH_AUTO_REBOOT
    // virtual
    bool bilgeAlarm::okToAutoReboot() // override;
        // it is only ok to auto reboot if nothing
        // is going on, and hasn't for at least 5 mintues
    {
        return !_state &&
               !_alarm_state &&
               time(NULL) - _time_last_run > 300;
    }
#endif

//--------------------------------
// implementation
//--------------------------------

bilgeAlarm::bilgeAlarm()
{
    bilge_alarm = this;
    addValues(m_bilge_values,NUM_BILGE_VALUES);
    setTabLayouts(dash_items,config_items);
}

// void bilgeAlarm::onInitRTCMemory()
// {
//     LOGI("bilgeAlarm::onInitRTCMemory()");
//     ba_history.initRTCMemory();
//     myIOTDevice::onInitRTCMemory();
// }


void bilgeAlarm::setup()
{
    LOGD("bilgeAlarm::setup(%s) started",getVersion());
    proc_entry();

    pinMode(PIN_ALARM,OUTPUT);
    pinMode(PIN_RELAY,OUTPUT);
    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_RELAY,0);

    pinMode(PIN_PUMP1_ON, INPUT_PULLDOWN);
    pinMode(PIN_PUMP2_ON, INPUT_PULLDOWN);
    // pinMode(PIN_5V_IN,    INPUT);
    // pinMode(PIN_12V_IN,   INPUT);

    int button_pins[] = {PIN_BUTTON0,PIN_BUTTON1,PIN_BUTTON2};
    // ui_screen =
    new uiScreen(button_pins);

    showIncSetupPixel();    // 1

    // init myIOTDevice

    myIOTDevice::setup();

    // init hisotry
    // if there was any history, setup the in-memory variables to be published

    ba_history.initHistory();

    int iter = 0;
    ba_history.initIterator(&iter);
    const runHistory_t *hist = ba_history.getNext(&iter);
    if (hist)
    {
        LOGD("initializing last_run to %s dur %d",timeToString(hist->tm).c_str(),hist->dur);
        _time_last_run = hist->tm;
        _since_last_run = (int32_t) hist->tm;
        _dur_last_run = hist->dur;
    }

    _history_link = "<a href='/custom/getHistory?uuid=";
    _history_link += getUUID();
    _history_link += "' target='_blank'>History</a>";

    // finish bilgeAlarm setup
    //---------------------------------
    // See core notes in myIOTTypes.h
    // Since I have removed all direct or indirect calls to setXXX()
    // from the stateTask() call chain, this task may now run on
    // ESP32_CORE_OTHER==0, separate from loop(), and the serial and
    // websocket task which must all run on ESP32_CORE_ARDUINO==1
    // for the LCD to work correctly.

    setPixelBright(0,getInt(ID_LED_BRIGHT));
    setPixelBright(1,getInt(ID_EXT_LED_BRIGHT));
    startAlarm();


    LOGI("starting stateTask");
    xTaskCreatePinnedToCore(stateTask,
        "stateTask",
        4096,           // task stack
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL,           // returned task handle
        ESP32_CORE_OTHER);

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
        LOGD("%s(0x%04x=%s) => 0x%04x=%s",
            what,
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
    debugState("setState",state,state,systemStates);
    _state = state;
}
void bilgeAlarm::addState(uint32_t state)
{
    uint32_t new_state = _state | state;
    debugState("addState",state,new_state,systemStates);
    _state = new_state;

}
void bilgeAlarm::clearState(uint32_t state)
{
    uint32_t new_state = _state & ~state;
    debugState("clearState",state,new_state,systemStates);
    _state = new_state;}

void bilgeAlarm::setAlarmState(uint32_t alarm_state)
{
    debugState("setAlarmState",alarm_state,alarm_state,alarmStates);
    _alarm_state = alarm_state;
}
void bilgeAlarm::addAlarmState(uint32_t alarm_state)
{
    uint32_t new_state = _alarm_state | alarm_state;
    debugState("addAlarmState",alarm_state,new_state,alarmStates);
    _alarm_state = new_state;
}
void bilgeAlarm::clearAlarmState(uint32_t alarm_state)
{
    uint32_t new_state = _alarm_state & ~alarm_state;
    debugState("clearAlarmState",alarm_state,new_state,alarmStates);
    _alarm_state = new_state;
}



//---------------------------------------
// commands
//---------------------------------------
// called from UI and myIOTValue.cpp invoke() via registered fxn


void bilgeAlarm::selfTest()
    // i don't like to run the pump during selfTest because because that
    // interferes with statistical measurements of what's actually going
    // on in the bilge.
{
    LOGU("selfTest()");
    m_in_self_test = 1;
    // digitalWrite(PIN_RELAY,1);
    alarmSelfTest();
    // digitalWrite(PIN_RELAY,0);
    // delay(1000);
    m_in_self_test = 0;
}


void bilgeAlarm::suppressAlarm()
{
    LOGU("suppressAlarm()");
    addAlarmState(ALARM_STATE_SUPPRESSED);
}

void bilgeAlarm::clearHistory()
{
    LOGU("clearHistory()");
    clearError();
    ba_history.clearHistory();
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
            ba_history.endRun();
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
            ba_history.setCountWindow(COUNT_HOUR,_err_per_hour - 1);
        if (_state & STATE_TOO_OFTEN_DAY)
            ba_history.setCountWindow(COUNT_DAY,_err_per_day - 1);

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



void bilgeAlarm::onLedBright(const myIOTValue *desc, bool val)
{
    setPixelBright(strcmp(desc->getId(),ID_LED_BRIGHT),val);
}


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
        return ba_history.getHistoryHTML();
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
        #if DEBUG_SWITCHES
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        #else
            vTaskDelay(bilge_alarm->_sense_millis / portTICK_PERIOD_MS);
        #endif
        if (!m_in_self_test)
            bilge_alarm->stateMachine();
    }
}



void bilgeAlarm::stateMachine()
    // THIS IS THE STATE MACHINE FOR THE BILGE ALARM
    // DO NOT CALL setXX directly or indirectly from this!!!
    //
    // There is an issue that we don't want to publish to WS or MQTT in the middle of this
    // due to timing delay, but as important, the LCD wont work if we do any setXXX() value
    // changes from here and this happens to be running on Core 0 ...
{
    uint32_t now = millis();
    time_t time_now = time(NULL);

    int raw1 = 0;
    int raw2 = 0;

    #if 0
        bool pump1_on = digitalRead(PIN_PUMP1_ON);
        bool pump2_on = digitalRead(PIN_PUMP2_ON);
    #else
        raw1 = analogRead(PIN_PUMP1_ON);
        raw2 = analogRead(PIN_PUMP2_ON);
        bool pump1_on = raw1 > _sw_threshold ? 1 : 0;
        bool pump2_on = raw2 > _sw_threshold ? 1 : 0;
    #endif

    #if DEBUG_SWITCHES
        LOGD("DEBUG_SWITCHES   pump1=%-6d  pump2=%-6d",raw1,raw2);
        return;
    #endif


    bool was_on1 = _state & STATE_PUMP1_ON;
    bool was_on2 = _state & STATE_PUMP2_ON;
    bool pump1_valid = now > m_pump1_debounce_time;
    bool pump2_valid = now > m_pump2_debounce_time;
    bool forced_on = _state & STATE_RELAY_FORCED;
    bool emergency_relay_on = _state & STATE_RELAY_EMERGENCY;
    bool after_extra = _extra_run_mode && _state & STATE_RELAY_EXTRA;

    static uint32_t subtract_relay;

    // start the run

    if (!count_this_run)
    {
        if ((pump2_on && pump2_valid) ||
            (pump1_on && pump1_valid && !emergency_relay_on && !forced_on && !after_extra))
        {
            subtract_relay = 0;
            ba_history.startRun();
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
        LOGI("m_pump2_on=%d  raw2=%d",pump2_on,raw2);
        m_pump2_debounce_time = now  + _pump_debounce;

        if (pump2_on)
        {
            uint32_t new_state = _state | STATE_PUMP2_ON;
            bool was_emergency = _state & STATE_EMERGENCY;

            if (_disabled != ALARM_DISABLED)
            {
                new_state |= STATE_EMERGENCY;
                setAlarmState(ALARM_STATE_EMERGENCY | ALARM_STATE_CRITICAL);
                if (_run_emergency)
                {
                    new_state |= STATE_RELAY_EMERGENCY;
                    new_state &= ~(STATE_RELAY_EXTRA | STATE_RELAY_FORCED);
                }
            }

            setState(new_state);
            ba_history.setRunFlags(STATE_EMERGENCY);

            if (_disabled != ALARM_DISABLED)
            {
                if (!was_emergency)
                {
                    LOGU("ALARM - EMERGENCY PUMP ON!!");
                }
                if (_run_emergency && !emergency_relay_on)
                {
                    LOGU("EMERGENCY RELAY ON");
                    emergency_relay_on = 1;
                }
            }
        }
        else
        {
            // LOGW("clearing emergency _alarm_state=0x%02x",_alarm_state);
            clearState(STATE_PUMP2_ON);
            if (_alarm_state & ALARM_STATE_EMERGENCY)
            {
                LOGU("ALARM - EMERGENCY PUMP OFF - DOWNGRADE TO CRITICAL");
                clearAlarmState(ALARM_STATE_EMERGENCY);
            }
        }
    }

    // extend the emergency relay as long as the pump2 switch is on

    if (pump2_valid && pump2_on && emergency_relay_on)
    {
        m_relay_time = now + _run_emergency * 1000;
    }


    //-----------------------------
    // PUMP1
    //-----------------------------

    if (pump1_valid && was_on1 != pump1_on)
    {
        LOGI("m_pump1_on=%d  raw1=%d",pump1_on,raw1);
        m_pump1_debounce_time = now + _pump_debounce;

        if (pump1_on)
        {
            addState(STATE_PUMP1_ON);
        }
        else
        {
            clearState(STATE_PUMP1_ON);
        }

        if (_disabled != ALARM_DISABLED && !forced_on && !emergency_relay_on && _extra_run_time)
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

        int duration = time_now - ba_history.getStartDuration();
        duration -= subtract_relay;

        // raise duration based alarms

        if (_err_run_time && duration > _err_run_time)
        {
            ba_history.setRunFlags(STATE_TOO_LONG);
            if (_disabled != ALARM_DISABLED && !(_state & STATE_TOO_LONG))
            {
                LOGU("ALARM - PUMP TOO LONG");
                addState(STATE_TOO_LONG);
                addAlarmState(ALARM_STATE_ERROR);
            }
        }


        if (_crit_run_time && duration > _crit_run_time)
        {
            ba_history.setRunFlags(STATE_CRITICAL_TOO_LONG);
            if (_disabled != ALARM_DISABLED && !(_state & STATE_CRITICAL_TOO_LONG))
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

    if (_disabled != ALARM_DISABLED && !forced_on)
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
                else if (extra_on &&_extra_run_time)
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

        int count_hour = ba_history.countRuns(COUNT_HOUR);
        int count_day = ba_history.countRuns(COUNT_DAY);

        // not to do with errors, but we use the timer to also update the weekly count
        // EVERY SECOND .. could get slow ...

        _num_last_week = ba_history.countRuns(COUNT_WEEK);

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
                ba_history.setRunFlags(STATE_TOO_OFTEN_HOUR);

                if (_disabled != ALARM_DISABLED)
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
                ba_history.setRunFlags(STATE_TOO_OFTEN_DAY);

                if (_disabled != ALARM_DISABLED)
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
        _dur_last_run = ba_history.endRun();
        count_this_run = 0;
        subtract_relay = 0;
    }

}   // stateMachine()



void bilgeAlarm::publishState()
{
    // critical section
    uint32_t state = _state;
    uint32_t alarm_state = _alarm_state;
    time_t time_last_run = _time_last_run;
    int since_last_run = _since_last_run;
    int dur_last_run = _dur_last_run;
    int num_last_hour = _num_last_hour;
    int num_last_day = _num_last_day;
    int num_last_week = _num_last_week;
    // end critical section

    // bilgeAlarm.cpp (the state machine) can see the _member variables,
    // but everyone else can only see the m_publish_state members which
    // are set here ..

    if (m_publish_state.state != state)
    {
        // LOGD("bilgeAlarm publishing state 0x%04x",state);
        setBenum(ID_STATE,state);
    }
    if (m_publish_state.alarm_state != alarm_state)
    {
        // LOGD("bilgeAlarm publishing alarm_state 0x%04x",alarm_state);
        setBenum(ID_ALARM_STATE,alarm_state);
    }
    if (m_publish_state.time_last_run != time_last_run)
    {
        LOGD("bilgeAlarm publishing time_last_run %s",timeToString(time_last_run).c_str());
        setTime(ID_TIME_LAST_RUN,time_last_run);
    }
    if (m_publish_state.since_last_run != since_last_run)
    {
        LOGD("bilgeAlarm publishing since_last_run %d",since_last_run);
        setInt(ID_SINCE_LAST_RUN,since_last_run);
    }
    if (m_publish_state.dur_last_run != dur_last_run)
    {
        // LOGD("bilgeAlarm publishing dur_last_run %d",dur_last_run);
        setInt(ID_DUR_LAST_RUN,dur_last_run);
    }
    if (m_publish_state.num_last_hour != num_last_hour)
    {
        setInt(ID_NUM_LAST_HOUR,num_last_hour);
    }
    if (m_publish_state.num_last_day != num_last_day)
    {
        setInt(ID_NUM_LAST_DAY,num_last_day);
    }
    if (m_publish_state.num_last_week != num_last_week)
    {
        setInt(ID_NUM_LAST_WEEK,num_last_week);
    }
}



//-----------------------------------
// power stuff
//-----------------------------------
#include <esp_adc_cal.h>

#define VOLT_OFFSET_12V  0.00
#define VOLT_OFFSET_5V   -0.05

#define DIVIDER_RATIO_12V    ((4760.0 + 1000.0)/1000.0)       // (R1 + R2)/R2   ~5.6, so 3.3V==18.5V               (mesasured but wrong supply)
    // As it stands right now, with no offsets,
    // the display is matching the power supply pretty well
    // power supply      multi meter      values
    //    12.9             12.78         12.9
    //    11.5             11.36         11.5
    // The voltage will fluctuate pretty wildly esp in test harness
    // with alarm, relay, etc.

#define DIVIDER_RATIO_5V     ((10000.0 + 10000.0)/10000.0)    // (R1 + R2)/R2   ~2, so 3.3V==6.6V (we need 0..5)   (unmeasured)                                    ))
    // Measuring 5.03 with multi-meter while displaying 5.08 == 5.1
    // so I subtract an offset of 0.05

#define NUM_VOLT_SAMPLES     100
#define CHECK_POWER_INTERVAL 100
    // take a samples and
    // publish new value every so often


int sample_5v;
int sample_12v;
int sample_counter;


static void getPower(bool v5, valueIdType id, int val, float cur, int calib)
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1200, &adc_chars);
        // prh dunno what it does, but it's way better than analogRead()/4096 * 3.3V
        // The 1100 is from https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
        // The constant does not seem to do anything
    uint32_t raw_millis = esp_adc_cal_raw_to_voltage(val, &adc_chars);
    float raw_volts = (float) raw_millis/1000.0;

    // throw in fixed ratios and offsets

    float undivided_volts = raw_volts * (v5 ? DIVIDER_RATIO_5V : DIVIDER_RATIO_12V);
    float final_volts = undivided_volts + (v5 ? VOLT_OFFSET_5V : VOLT_OFFSET_12V);

    // throw in parameterized ratio

    float user_ratio = calib;
    user_ratio /= 1000.00;
    final_volts *= user_ratio;

    // round to 1 decimal place

    uint32_t rounded = 1000 * final_volts;
    rounded = (rounded + 50) / 100;

    float final = rounded;
    final /= 10;

    #if 0   // DEBUG_POWER
        LOGD("%s val=%d   millis=%d   raw==%-2.3f   undiv=%-2.3f   volts=%-2.3f  rounded=%d  final=%-2.3f",
                (v5 ? "CPU" : "MAIN"),
                val,
                raw_millis,
                raw_volts,
                undivided_volts,
                final_volts,
                rounded,
                final);
    #endif

    if (final != cur)
    {
        bilge_alarm->setFloat(id,final);
    }
}


void bilgeAlarm::checkPower()
{
    uint32_t now = millis();
    static uint32_t check_power_time = 0;
    if (!check_power_time || now > check_power_time + CHECK_POWER_INTERVAL)
    {
        check_power_time = now;

        // don't check the power if the alarm or relay is on

        if (!_alarm_state &&
            !m_publish_state.alarm_state &&
            !(_state & STATE_RELAY_ON) &&
            !(m_publish_state.state & STATE_RELAY_ON))
        {
            sample_12v += analogRead(PIN_12V_IN);
            sample_5v += analogRead(PIN_5V_IN);
            sample_counter++;
            if (sample_counter == NUM_VOLT_SAMPLES)
            {
                sample_12v /= NUM_VOLT_SAMPLES;
                sample_5v /= NUM_VOLT_SAMPLES;
                getPower(0,ID_POWER_12V,sample_12v,_power_12v,_calib_12v);
                getPower(1,ID_POWER_5V,sample_5v,_power_5v,_calib_5v);
                sample_counter = 0;
                sample_12v = 0;
                sample_5v = 0;
            }
        }
    }
}


//-----------------------------------
// loop()
//-----------------------------------

void bilgeAlarm::loop()
{
    myIOTDevice::loop();
    bilge_alarm->publishState();
    bilge_alarm->checkPower();
    ui_screen->loop();
}
