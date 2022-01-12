//-----------------------------------
// bilgeAlarm.cpp
//-----------------------------------
#include "bilgeAlarm.h"
#include "baScreen.h"
#include <myIOTLog.h>


#define DEBUG_STATES  1
#define DEBUG_ALARM   0
#define DEBUG_COUNTS  0


#define TEST_TIMES  1
    // define allows me to test in compressed time where
    // one hour == 1 minute, one day=3 minutes, and one week==5 minutes

#define BUTTON_CHECK_TIME  30
#define UI_UPDATE_TIME     30


#define MAX_RUN_HISTORY    256      // 256 * 8 = 2K
    // set to smaller number (i.e 15) for testing circular buffer
    //
    // circular buffer of run history, where each history element
    // is 8 bytes (two 32 bit numbers), the first being the time
    // of the run, and the second being the duration of the run in seconds.
    // The buffer MUST be large enough to handle all the runs in a day,
    // or certainly at least more than the error configuration values

#define COUNT_HOUR   0
#define COUNT_DAY    1
#define COUNT_WEEK   2
    // the type of count to do

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
static enumValue pumpExtraType[] = {
    "FROM_START",
    "AFTER_END",
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

    { ID_DISABLED,         VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_disabled,      (void *) onDisabled,   { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_backlight_secs, NULL,  { .int_range = { DEFAULT_BACKLIGHT_SECS,    0,  3600}}  },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_run_time,   NULL,  { .int_range = { DEFAULT_ERR_RUN_TIME,      0,  3600}}  },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_crit_run_time,  NULL,  { .int_range = { DEFAULT_CRIT_RUN_TIME,     0,  3600}}  },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_per_hour,   NULL,  { .int_range = { DEFAULT_ERR_PER_HOUR,      0,  MAX_RUN_HISTORY}}   },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_err_per_day,    NULL,  { .int_range = { DEFAULT_ERR_PER_DAY,       0,  MAX_RUN_HISTORY}}   },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_run_emergency,  NULL,  { .int_range = { DEFAULT_RUN_EMERGENCY,     0,  3600}}  },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_extra_run_time, NULL,  { .int_range = { DEFAULT_EXTRA_RUN_TIME,    0,  3600}}  },
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

time_t bilgeAlarm::m_start_duration;
    // The "duration" that pump1 is "on"
    // INCLUDES the "extra_time" if EXTRA_MODE(0) "at_start", so
    // the ERR_RUN_TIME and CRIT_RUN_TIME values must be larger than EXTRA_RUN_TIME
    // if using EXTRA_RUN_MODE(0) "at_start"
time_t bilgeAlarm::m_clear_day_time;
time_t bilgeAlarm::m_clear_hour_time;
    // if a clearError is done, these prevents countRuns() from returning
    // any runs before this time.


typedef struct {
    time_t  tm;
    int     dur;
} runHistory_t;


static runHistory_t run_history[MAX_RUN_HISTORY];
static int run_head = 0;


bilgeAlarm *bilge_alarm = NULL;
// iotLCD_UI *my_ui = NULL;




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
    // ba_screen =
    new baScreen(button_pins);

    myIOTDevice::setup();

    LOGI("starting alarmTask");
    xTaskCreate(alarmTask,
        "alarmTask",
        DEBUG_ALARM ? 4096 : 1024,  // very small stack unless debuggin
        NULL,           // param
        5,  	        // note that the priority is higher than one
        NULL);

    _history_link = "<a href='/custom/getHistory?uuid=";
    _history_link += getUUID();
    _history_link += "' target='_blank'>History</a>";

    proc_leave();
    LOGD("bilgeAlarm::setup(%s) completed",getVersion());


}


//-----------------------------------------------
// in-memory list of pump_runs (database)
//-----------------------------------------------

void bilgeAlarm::startRun()
{
    time_t now = time(NULL);
    m_start_duration = now;
    LOGU("PUMP ON(%d)",run_head);
    runHistory_t *ptr = &run_history[run_head];
    ptr->tm = now;
    ptr->dur = 0;
    setTime(ID_TIME_LAST_RUN,now);
    setInt(ID_SINCE_LAST_RUN,(int32_t)now);
    setInt(ID_DUR_LAST_RUN,0);
}

void bilgeAlarm::endRun()
{
    time_t now = time(NULL);
    int duration = now - m_start_duration;
    if (duration == 0) duration = 1;
    LOGU("PUMP OFF(%d) %d secs",run_head,duration);
    runHistory_t *ptr = &run_history[run_head];
    ptr->dur = duration;
    setInt(ID_DUR_LAST_RUN,duration);
    m_start_duration = 0;
    if (++run_head >= MAX_RUN_HISTORY)
        run_head = 0;
}


int bilgeAlarm::countRuns(int type)
    // note that this does NOT count the current run which is happening
    // it only counts fully commited runs, so per_day and per_hour alarms
    // happen when the pump turns OFF.   Due to the alarm clearing strategy,
    // the and week counts shown after a clear will more or less be zeroed,
    // though the history still contains all the runs.
{
    // LOGD("countRuns(%d) run_head=%d",hours,run_head);
    // can't really debug in here since it is called every second

    uint32_t cutoff;
    time_t now = time(NULL);

    if (type == COUNT_WEEK)
    {
        cutoff = now - (TEST_TIMES ? (5 * 60) : (7 * 24 * 60 * 60));
    }
    else if (type == COUNT_DAY)
    {
        cutoff = now - (TEST_TIMES ? (3 * 60) : (24 * 60 * 60));
        if (m_clear_day_time > cutoff)
            cutoff = m_clear_day_time;
    }
    else // type == COUNT_HOUR
    {
        cutoff = now - (TEST_TIMES ? (1 * 60) : (60 * 60));
        if (m_clear_hour_time > cutoff)
            cutoff = m_clear_hour_time;
    }

    int count = 0;
    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];

    while (ptr->tm && ptr->tm >= cutoff)
    {
        count++;
        // LOGD("    Counting run(%d) at %s",count,timeToString(ptr->tm).c_str());
        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    // LOGD("countRuns(%d) returning %d",hours,count);
    return count;
}


static time_t countBackRuns(int num)
{
    #if DEBUG_COUNTS
        LOGD("countBackRuns(%d)",num);
    #endif

    int count = 0;
    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];

    // so if the error is set to 4 per hour, we want to return the
    // time of the 4th one back

    while (ptr->tm)
    {
        count++;
        if (count == num)
        {
            #if DEBUG_COUNTS
                LOGD("    countBackRuns(%d) returning(%d) at %s",num,count,timeToString(ptr->tm).c_str());
            #endif
            return ptr->tm;
        }

        #if DEBUG_COUNTS
            LOGD("    countBackRuns(%d) skipping(%d) at %s",num,count,timeToString(ptr->tm).c_str());
        #endif

        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    // should never get here, since the history just triggered an error

    return 0;
}


// might be a stack overflow
// building this on the stack

static String historyHTML()
{
    String rslt = "<head>\n";
    rslt += "<title>";
    rslt += bilge_alarm->getName();
    rslt += " History</title>\n";
    rslt += "<body>\n";
    rslt += "<style>\n";
    rslt += "th, td { padding-left: 12px; padding-right: 12px; }\n";
    rslt += "</style>\n";

    // this loop should be abstracted

    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];
    int count = 0;

    while (ptr->tm)
    {
        count++;
        if (count == 1)
        {
            rslt += "<b>";
            rslt +=  bilge_alarm->getName();
            rslt += " History</b><br><br>\n";
            rslt += "<table border='1' padding='6' style='border-collapse:collapse'>\n";
            rslt += "<tr><th>num</th><th>time</th><th>dur</th><th>ago</th></tr>\n";
        }

        rslt += "<tr><td>";
        rslt += String(count);
        rslt += "  (";
        rslt += String(head);
        rslt += ")";
        rslt += "</td><td>";
        rslt += timeToString(ptr->tm);
        rslt += "</td><td align='center'>";
        rslt += String(ptr->dur);
        rslt += "</td><td>";

        // anything over a 5 years year is considered invalid to deal with potential lack of clock issues

        char buf[128] = "&nbsp;";
        uint32_t since = time(NULL) - ptr->tm;
        if (since < 5 * 365 * 24 * 60 * 60)
        {
            int days = since / (24 * 60 * 60);
            int hours = (since % (24 * 60 * 60)) / (60 * 60);
            int minutes = (since % (24 * 60 * 60 * 60)) / 60;
            int secs = since % 60;
            if (days)
            {
                sprintf(buf,"%d days  %02d:%02d:%02d",days, hours, minutes, secs);
            }
            else
            {
                sprintf(buf,"%02d:%02d:%02d",hours, minutes, secs);
            }
        }
        rslt += buf;
        rslt += "</td></tr>\n";

        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    if (!count)
        rslt += "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    else
        rslt += "</table>";

    rslt += "</body>\n";
    return rslt;
}



String bilgeAlarm::onCustomLink(const String &path)
{
    if (path.startsWith("getHistory"))
    {
        return historyHTML();
    }
    return "";
}


//------------------------------------
// methods
//------------------------------------

void bilgeAlarm::suppressAlarm()
{
    LOGU("suppressAlarm()");
    setAlarmState(_alarm_state | ALARM_STATE_SUPPRESSED);
}


void bilgeAlarm::clearError()
{
    LOGU("clearError()");
    if (_alarm_state)
    {
        setAlarmState(0);

        // when we clear either of these errors, we set the time window
        // so that the count goes down to ONE UNDER the error limit,
        // to prevent false retriggers, but allow the next pump run
        // to re-trigger the error.  Changing the error parameters,
        // clearing the history, and disabling the alarm are other
        // user strategies to cope with pesky count alarms.
        //
        // hopefully the count will roll off before the pump goes back on

        if (_state & STATE_TOO_OFTEN_HOUR)
        {
            m_clear_hour_time = countBackRuns(_err_per_hour - 1);
            LOGU("Setting m_clear_hour_time to %s",timeToString(m_clear_hour_time));
        }
        if (_state & STATE_TOO_OFTEN_DAY)
        {
            m_clear_day_time = countBackRuns(_err_per_day - 1);
            LOGU("Setting m_clear_day_time to %s",timeToString(m_clear_day_time));
        }

        setState(0);

        // we manually force the relay off in case it is on

        digitalWrite(PIN_RELAY,0);
    }
}


void bilgeAlarm::clearHistory()
{
    LOGU("clearHistory()");
    clearError();
    memset(run_history,0,MAX_RUN_HISTORY * sizeof(runHistory_t));
    run_head = 0;
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


void bilgeAlarm::onValueChanged(const myIOTValue *value)
{
    ba_screen->onValueChanged(value);
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


void bilgeAlarm::onDisabled(const myIOTValue *value, bool val)
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
        LOGU("FORCE RELAY ON");
        new_state &= ~(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA);
        new_state |= STATE_RELAY_FORCED | STATE_RELAY_ON;
    }
    else
    {
        LOGU("FORCE RELAY OFF");
        m_suppress_next_after = 1;
        new_state &= ~(STATE_RELAY_FORCED | STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
    }

    digitalWrite(PIN_RELAY,val);
    if (new_state != _state)
        setState(new_state);
}



void bilgeAlarm::handleSensors()
    // THIS IS THE STATE MACHINE FOR THE BILGE ALARM
    // There is an issue that we don't want to publish to WS or MQTT in the middle of time,
    // but I am going to ignore that for now and may eventually implement an asynchronous
    // publishing task in myIOTValues.cpp
{
    uint32_t now = millis();
    time_t time_now = time(NULL);
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
        LOGI("m_pump2_on=%d",pump2_on);
        m_pump2_debounce_time = now  + _pump_debounce;

        if (pump2_on)
        {
            new_state |= STATE_PUMP2_ON;
            if (!_disabled &&
                !(new_state & STATE_EMERGENCY))
            {
                LOGU("ALARM - EMERGENCY PUMP ON!!");
                new_state |= STATE_EMERGENCY;
                new_alarm_state |= ALARM_STATE_CRITICAL | ALARM_STATE_EMERGENCY;
            }
        }
        else
        {
            new_state &= ~STATE_PUMP2_ON;
            if (!_disabled && (new_alarm_state & ALARM_STATE_EMERGENCY))
            {
                LOGU("ALARM - EMERGENCY PUMP OFF - DOWNGRADE TO CRITICAL");
                new_alarm_state &= ~ALARM_STATE_EMERGENCY;
            }
        }

        if (!_disabled && !forced_on && pump2_on &&_run_emergency)
        {
            LOGU("EMERGENCY RELAY ON");
            new_state &= ~STATE_RELAY_EXTRA;
            new_state |= STATE_RELAY_EMERGENCY;
            emergency_on = 1;
        }
    }

    // extend the emergency relay as long as the pump2 switch is on

    if (pump2_on && emergency_on)
    {
        m_relay_time = now + _run_emergency * 1000;
    }


    //-----------------------------
    // PUMP1
    //-----------------------------

    bool pump1_turned_on = false;

    if (was_on1 != pump1_on && now > m_pump1_debounce_time)
    {
        LOGI("m_pump1_on=%d",pump1_on);
        m_pump1_debounce_time = now + _pump_debounce;
        pump1_turned_on = pump1_on;

        static bool count_this_run = 0;

        if (pump1_on)
        {
            new_state |= STATE_PUMP1_ON;

            // ok, the tricky part
            // we definitely don't want to do a 'run' if it was forced_on.
            // if the relay is on for extra_time, that implies it went on already once,
            //    but we don't want to count the second run.
            // and, by rights, it's probably ok to NOT count the run if it's due to
            //    run_emergency.
            // so, we don't want to start a run here if the relay is on.

            if (!(new_state & STATE_RELAY_ON))
            {
                count_this_run = 1;
                bilge_alarm->startRun();
            }
        }
        else
        {
            // and we only want to end a run if we started one.
            // the tricky case is when the relay goes off but the pump stays on.
            // what we are going to do is we will only end the run IFF we started it.

            new_state &= ~STATE_PUMP1_ON;
            if (count_this_run)
            {
                count_this_run = 0;
                bilge_alarm->endRun();
            }
        }

        if (!_disabled && !forced_on && !emergency_on && _extra_run_time)
        {
            if (pump1_on && !_extra_run_mode)
            {
                LOGU("EXTRA RELAY ON");
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

    if (pump1_on)
    {
        if (m_start_duration)
        {
            // as long as the relay is on (we are assuming the relay
            // is connected to the pump switch which we are sampling),
            // we advance the duration starting time to prevent time
            // related errors due to the relay itself.

            if (new_state & STATE_RELAY_ON)
                m_start_duration = time_now;

            // calculate duration that the pump has been on

            uint32_t duration = time_now - m_start_duration;

            // raise duration based alarms

            if (_err_run_time && duration > _err_run_time &&
                !(new_state & STATE_TOO_LONG))
            {
                LOGU("ALARM - PUMP TOO LONG");
                new_state |= STATE_TOO_LONG;
                new_alarm_state |= ALARM_STATE_ERROR;
            }
            if (_crit_run_time && duration > _crit_run_time &&
                !(new_state & STATE_CRITICAL_TOO_LONG))
            {
                LOGU("ALARM - PUMP CRITICAL TOO LONG");
                new_state |= STATE_CRITICAL_TOO_LONG;
                new_alarm_state |= ALARM_STATE_CRITICAL;
            }
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
                // the LOGU's are such that we only show a message when
                // the kind of relay is turned on.

                if (should_be_on)
                {
                    new_state |= STATE_RELAY_ON;
                    if (m_relay_delay_time)
                        LOGU("DEFERRED EXTRA RELAY ON");
                    m_relay_delay_time = 0;
                }

                // I think this else is in case they disable the system
                // while there are relays on.

                else
                {
                    LOGU("DISABLED - RELAY_OFF");
                    new_state &= ~STATE_RELAY_ON;
                }

                // just for good measure I also output a LOGI showing the state change

                LOGI("relay=%d",should_be_on);
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
            LOGU("AUTO RELAY_OFF");
            m_relay_time = 0;
            m_suppress_next_after = 1;
            new_state &= ~(STATE_RELAY_EMERGENCY | STATE_RELAY_EXTRA | STATE_RELAY_ON);
            digitalWrite(PIN_RELAY,0);
        }
    }

    //---------------------------
    // count based alarms
    //---------------------------
    // the 'per hour' and 'per day' alarms are not triggered until the pump goes off
    // and the runs can be properly counted, and they are only triggered if not already
    // set and the counts change.  They are done after the relay logic for sanity.
    // They self clear if they are the only error and the counters go down.
    // only need to do this once per second.
    //
    // NOTE THAT TO PREVENT NEW ALARMS OF THE SAME TYPE ON ROLL OFF OF PREVIOUS TIMES,
    // which triggers the 'changed' logic below,
    // we set a "window" on countRuns() per error type when clearing the error, and
    // henceforth the counts for those specific error types will NOT REFLECT THE
    // ACTUAL COUNT, EVEN THOUGH THE HISTORY STILL CONTAINS THEM.
    // Otherwise, if the counts remain above the set levels after clearing,
    // this logic would re-assert the error on a change to the counts
    // (i.e. when one rolls off) EVEN IF THE PUMP DOESNT GO BACK ON.

    static time_t last_clock_time = 0;
    if (time_now != last_clock_time)
    {
        last_clock_time = time_now;

        bool clear_this_error = 0;
        int count_hour = bilge_alarm->countRuns(COUNT_HOUR);
        int count_day = bilge_alarm->countRuns(COUNT_DAY);
        bool only_run_errors =
            !(new_alarm_state & ~(ALARM_STATE_ERROR | ALARM_STATE_SUPPRESSED)) &&
            !(new_state & ~(STATE_TOO_OFTEN_HOUR | STATE_TOO_OFTEN_DAY));

        if (count_hour != _num_last_hour)
        {
            setInt(ID_NUM_LAST_HOUR,count_hour);

            #if DEBUG_COUNTS
                LOGD("count_hour(%d) only(%d) _disabled(%d) _err_per(%d) _num(%d) state(0x%04x) alarm(0x%02x)",
                     count_hour,
                     only_run_errors,
                     _disabled,
                     _err_per_hour,
                     _num_last_hour,
                     new_state,
                     new_alarm_state);
            #endif

            if (!_disabled &&
                _err_per_hour &&
                _num_last_hour >= _err_per_hour &&
                !(new_state & STATE_TOO_OFTEN_HOUR))
            {
                LOGU("ALARM - TOO MANY PER HOUR");
                new_state |= STATE_TOO_OFTEN_HOUR;
                new_alarm_state |= ALARM_STATE_ERROR;
            }
            else if (only_run_errors &&
                     (new_state & STATE_TOO_OFTEN_HOUR) &&
                     _num_last_hour < _err_per_hour)
            {
                LOGU("ALARM - SELF CLEARING TOO MANY PER HOUR");
                new_state &= ~STATE_TOO_OFTEN_HOUR;
                clear_this_error = 1;
            }
        }
        if (count_day != _num_last_day)
        {
            setInt(ID_NUM_LAST_DAY,count_day);

            #if DEBUG_COUNTS
                LOGD("count_day(%d) only(%d) _disabled(%d) _err_per(%d) _num(%d) state(0x%04x) alarm(0x%02x)",
                     count_day,
                     only_run_errors,
                     _disabled,
                     _err_per_day,
                     _num_last_day,
                     new_state,
                     new_alarm_state);
            #endif

            if (!_disabled &&
                _err_per_day &&
                _num_last_day >= _err_per_day &&
                !(new_state & STATE_TOO_OFTEN_DAY))
            {
                LOGU("ALARM - TOO MANY PER DAY");
                new_state |= STATE_TOO_OFTEN_DAY;
                new_alarm_state |= ALARM_STATE_ERROR;
            }
            else if (only_run_errors &&
                     (new_state & STATE_TOO_OFTEN_DAY) &&
                     _num_last_day < _err_per_day)
            {
                LOGU("ALARM - SELF CLEARING TOO MANY PER DAY");
                new_state &= ~STATE_TOO_OFTEN_DAY;
                clear_this_error = 1;
            }
        }
        if (clear_this_error &&
            only_run_errors &&
            new_alarm_state &&
            !new_state)
        {
            LOGU("ALARM - SELF CLEARING ALARM_STATE_COUNT");
            clearError();
            new_alarm_state = _alarm_state;
            new_state = _state;
        }

        // not to do with errors, but we use the timer to also update the weekly count
        // EVERY SECOND .. could get slow ...

        int count_week = bilge_alarm->countRuns(COUNT_WEEK);
        if (count_week != _num_last_week)
            setInt(ID_NUM_LAST_WEEK,count_week);
    }

    //------------------------------
    // publish
    //------------------------------

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
    static uint32_t check_sensors = 0;
    if (now > check_sensors + _sense_millis)
    {
        check_sensors = now;
        handleSensors();
    }

    ba_screen->loop();
}
