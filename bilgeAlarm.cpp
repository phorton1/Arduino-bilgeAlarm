//-----------------------------------
// bilgeAlarm.cpp
//-----------------------------------
#include "bilgeAlarm.h"
#include <myIOTLog.h>
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_LINE_LEN   16


//--------------------------------
// bilgeAlarm definition
//--------------------------------

#define DEFAULT_DISABLED            0          // enabled,disabled
#define DEFAULT_BACKLIGHT_SECS      0          // off,secs
#define DEFAULT_ERR_RUN_TIME        10         // off,secs
#define DEFAULT_CRIT_RUN_TIME       30         // off,secs
#define DEFAULT_ERR_PER_HOUR        2          // off,num
#define DEFAULT_ERR_PER_DAY         12         // off,secs
#define DEFAULT_EXTRA_RUN_TIME      5          // off,secs
#define DEFAULT_EXTRA_RUN_MODE      1          // start, end, if primary_time
#define DEFAULT_END_RUN_DELAY       2          // secs if mode=='end' and time != 0
#define DEFAULT_RUN_EMERGENCY       30         // 255


static valueIdType dash_items[] = {
    ID_STATE,
    ID_ALARM_STATE,
    ID_LCD_LINE1,
    ID_LCD_LINE2,
#ifdef WITH_POWER
    ID_DEVICE_VOLTS,
    ID_DEVICE_AMPS,
#endif
    ID_RELAY,
    ID_ONBOARD_LED,
    ID_OTHER_LED,
    ID_DEMO_MODE,
    ID_REBOOT,
    0
};

static valueIdType device_items[] = {
    ID_DEMO_MODE,
    ID_DISABLED,
    ID_BACKLIGHT_SECS,
    ID_ERR_RUN_TIME,
    ID_CRIT_RUN_TIME,
    ID_ERR_PER_HOUR,
    ID_ERR_PER_DAY,
    ID_EXTRA_RUN_TIME,
    ID_EXTRA_RUN_MODE,
    ID_END_RUN_DELAY,
    ID_RUN_EMERGENCY,
    0
};


static enumValue alarmStates[] = {"ERROR","CRITICAL","EMERGENCY","SUPRESSED",0};
static enumValue systemStates[] = {"PUMP_ON","RELAY_ON","FORCE_ON","RELAY_EMERGENCY","EMERGENCY_RUN","EMERGENCY_ON","TOO_OFTEN_HOUR","TOO_OFTEN_DAY","TOO_LONG","CRIT_LONG",0};


const valDescriptor bilgeAlarm::m_bilge_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,   VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,                   NULL,           "bilgeAlarm" },        // override base class element

    { ID_STATE,            VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_READONLY,   (void *) &m_state,      NULL,   { .enum_range = { 4, systemStates }} },
    { ID_ALARM_STATE,      VALUE_TYPE_BENUM,    VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_alarm_state,NULL,   { .enum_range = { 4, alarmStates }} },

    { ID_RELAY,            VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_RELAY,      (void *)onRelay },

    { ID_DISABLED,         VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_BACKLIGHT_SECS, 0, 255}} },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_RUN_TIME, 0, 255}} },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_CRIT_RUN_TIME, 0, 255}} },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_PER_HOUR, 0, 255}} },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_PER_DAY, 0, 255}} },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_EXTRA_RUN_TIME, 0, 255}} },
    { ID_EXTRA_RUN_MODE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_EXTRA_RUN_MODE, 0, 1}} },
    { ID_END_RUN_DELAY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_END_RUN_DELAY, 0, 255}} },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .float_range = {0, -1233.456, 1233.456}} },  // int_range = { DEFAULT_RUN_EMERGENCY, 0, 255}} },

    { ID_LCD_LINE1,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_lcd_line1,  (void *) onLcdLine },
    { ID_LCD_LINE2,        VALUE_TYPE_STRING,   VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_lcd_line2,  (void *) onLcdLine },

    { ID_ONBOARD_LED,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_ONBOARD_LED,(void *) onLed, },
    { ID_OTHER_LED,        VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_NONE,       (void *) &m_OTHER_LED,  (void *) onLed, },
    { ID_DEMO_MODE,        VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &m_DEMO_MODE,  NULL,           },
};

#define NUM_BILGE_VALUES (sizeof(m_bilge_values)/sizeof(valDescriptor))


uint32_t bilgeAlarm::m_state = 0;
uint32_t bilgeAlarm::m_alarm_state = 0;

String bilgeAlarm::m_lcd_line1;
String bilgeAlarm::m_lcd_line2;

bool bilgeAlarm::m_RELAY = 0;

bool bilgeAlarm::m_ONBOARD_LED = 0;
bool bilgeAlarm::m_OTHER_LED = 0;
bool bilgeAlarm::m_DEMO_MODE = 0;

bilgeAlarm *bilge_alarm = NULL;


LiquidCrystal_I2C lcd(0x27,LCD_LINE_LEN,2);   // 20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


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
    digitalWrite(PIN_ALARM,0);
    digitalWrite(PIN_RELAY,0);
    digitalWrite(PIN_ONBOARD_LED,0);

    pinMode(PIN_OTHER_LED,OUTPUT);
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

    proc_leave();
    LOGD("bilgeAlarm::setup(%s) completed",getVersion());
}


void bilgeAlarm::setState(uint32_t state)
{
    uint32_t new_state = m_state | state;
    bilge_alarm->setBenum(ID_STATE,new_state);
}
void bilgeAlarm::clearState(uint32_t state)
{
    uint32_t new_state = m_state & ~state;
    bilge_alarm->setBenum(ID_STATE,new_state);
}
void bilgeAlarm::setAlarmState(uint32_t alarm_state)
{
    uint32_t new_state = m_alarm_state | alarm_state;
    bilge_alarm->setBenum(ID_ALARM_STATE,new_state);
    LOGD("ALARM_STATE=0x%04x",m_alarm_state);
}
void bilgeAlarm::clearAlarmState(uint32_t alarm_state)
{
    uint32_t new_state = m_alarm_state & ~alarm_state;
    bilge_alarm->setBenum(ID_ALARM_STATE,new_state);
    LOGD("ALARM_STATE=0x%04x",m_alarm_state);
}


//---------------------------------------------------
// onXXX handlers - pins and hardware
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


void bilgeAlarm::onRelay(const myIOTValue *value, bool val)
{
    LOGD("onRelay(%d)",val);
    digitalWrite(PIN_RELAY,val);
}


void bilgeAlarm::onLcdLine(const myIOTValue *value, const char *val)
{
    int line = value->getId() == ID_LCD_LINE1 ? 1 : 0;
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



//---------------------------------------------
// loop handlers (state machine in pieces)
//---------------------------------------------

void bilgeAlarm::handleButtons()
{

}
void bilgeAlarm::handleSensors()
{
    bool pump1_on = digitalRead(PIN_PUMP1_ON);
    bool pump2_on = digitalRead(PIN_PUMP2_ON);
    bool was_on1 = m_state & STATE_PUMP_ON;
    bool was_on2 = m_state & STATE_EMERGENCY_PUMP_ON;

    if (pump1_on != was_on1)
    {
        LOGD("pump1_on=%d",pump1_on);
        changeState(pump1_on,STATE_PUMP_ON);
    }
    if (pump2_on != was_on2)
    {
        LOGD("pump2_on=%d",pump2_on);
        // of course this is just temporary state logic
        changeState(pump2_on,STATE_EMERGENCY_PUMP_ON);
        // which turns the alarm on directly ...
        changeAlarmState(pump2_on,ALARM_STATE_EMERGENCY);
    }
}



//-----------------------------------
// loop()
//-----------------------------------

void bilgeAlarm::loop()
{
    myIOTDevice::loop();

    uint32_t now = millis();
    static uint32_t check_buttons = 0;
    static uint32_t check_sensors = 0;
    if (now > check_buttons + 30)
    {
        check_buttons = now;
        handleButtons();
    }
    if (now > check_sensors + 300)
    {
        check_sensors = now;
        handleSensors();
    }

    static uint32_t last_alarm_state = 0;
    if (last_alarm_state != m_alarm_state)
    {
        last_alarm_state = m_alarm_state;
        bool on = m_alarm_state & ALARM_STATE_EMERGENCY;
        LOGD("write ALARM %d",on);
        digitalWrite(PIN_ALARM,on);
    }
}
