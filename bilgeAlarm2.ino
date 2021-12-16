

#include <myIOTDevice.h>
#include <myIOTLog.h>
#include <cstdlib>

#ifdef WITH_SD
    #define INIT_SD_EARLY
    #ifdef INIT_SD_EARLY
        #include <SD.h>
    #endif
#endif

#define BILGE_ALARM_VERSION "0.05"

#define ONBOARD_LED             2
#define OTHER_LED               13



class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm();
    ~bilgeAlarm() {}

    virtual const char *getVersion() override  { return BILGE_ALARM_VERSION; }

private:

    static const valDescriptor m_bilge_values[];

    static void onLed(const myIOTValue *desc, bool val);

// public:

    static bool m_ONBOARD_LED;
    static bool m_OTHER_LED;
    static bool m_DEMO_MODE;

};



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
#define ID_ONBOARD_LED      "ONBOARD_LED"
#define ID_OTHER_LED        "OTHER_LED"
#define ID_DEMO_MODE        "DEMO_MODE"


#define RETAINED_SWITCH    (VALUE_STYLE_SWITCH | VALUE_STYLE_RETAIN)
    // CAREFUL with the use of MQTT retained messages!!
    // They can only be cleared on the rpi with:
    //
    //      sudo service mosquitto stop
    //      sudo rm /var/lib/mosquitto/mosquitto.db
    //      sudo service mosquitto start
    //
    // or individually with
    //
    //      mosquitto_pub -u myIOTClient -P 1234 -h localhost -t bilgeAlarm/ONBOARD_LED -n -r -d

const valDescriptor bilgeAlarm::m_bilge_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,   VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,                   NULL,           "bilgeAlarm" },        // override base class element
    { ID_DISABLED,         VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { ID_BACKLIGHT_SECS,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_BACKLIGHT_SECS, 0, 255}} },
    { ID_ERR_RUN_TIME,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_RUN_TIME, 0, 255}} },
    { ID_CRIT_RUN_TIME,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_CRIT_RUN_TIME, 0, 255}} },
    { ID_ERR_PER_HOUR,     VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_PER_HOUR, 0, 255}} },
    { ID_ERR_PER_DAY,      VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_ERR_PER_DAY, 0, 255}} },
    { ID_EXTRA_RUN_TIME,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_EXTRA_RUN_TIME, 0, 255}} },
    { ID_EXTRA_RUN_MODE,   VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_EXTRA_RUN_MODE, 0, 1}} },
    { ID_END_RUN_DELAY,    VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .int_range = { DEFAULT_END_RUN_DELAY, 0, 255}} },
    { ID_RUN_EMERGENCY,    VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       NULL,                   NULL,           { .float_range = {0, -1233.456, 1233.456}} },  // int_range = { DEFAULT_RUN_EMERGENCY, 0, 255}} },
    { ID_ONBOARD_LED,      VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_SWITCH,     (void *) &m_ONBOARD_LED,(void *) onLed, },
    { ID_OTHER_LED,        VALUE_TYPE_BOOL,     VALUE_STORE_TOPIC,    VALUE_STYLE_SWITCH,     (void *) &m_OTHER_LED,  (void *) onLed, },
    { ID_DEMO_MODE,        VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_SWITCH,     (void *) &m_DEMO_MODE,  NULL,           },
};

#define NUM_BILGE_VALUES (sizeof(m_bilge_values)/sizeof(valDescriptor))


bool bilgeAlarm::m_ONBOARD_LED = 0;
bool bilgeAlarm::m_OTHER_LED = 0;
bool bilgeAlarm::m_DEMO_MODE = 0;

bilgeAlarm::bilgeAlarm()
{
    pinMode(ONBOARD_LED,OUTPUT);
    pinMode(OTHER_LED,OUTPUT);
    digitalWrite(ONBOARD_LED,0);
    digitalWrite(OTHER_LED,0);
    addValues(m_bilge_values,NUM_BILGE_VALUES);
}


void bilgeAlarm::onLed(const myIOTValue *value, bool val)
{
    String id = value->getId();
    // LOGD("onLed(%s,%d)",id.c_str(),val);

    if (id == ID_ONBOARD_LED)
    {
        digitalWrite(ONBOARD_LED,val);
    }
    else if (id == ID_OTHER_LED)
    {
        digitalWrite(OTHER_LED,val);
    }
}




//--------------------------------
// main
//--------------------------------

bilgeAlarm *bilge_alarm = NULL;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    LOGI("bilgeAlarm2.ino setup() started");

    pinMode(PIN_MAIN_VOLTAGE, INPUT);
    pinMode(PIN_MAIN_CURRENT, INPUT);

    #ifdef WITH_SD
    #ifdef INIT_SD_EARLY
        delay(200);
        bool sd_ok = SD.begin(5);     // 5
        LOGI("sd_ok=%d",sd_ok);
    #endif
    #endif

    proc_entry();

    my_iot_device = bilge_alarm = new bilgeAlarm();
    my_iot_device->setup();

    LOGI("%s version=%s IOTVersion=%s",
         my_iot_device->getName().c_str(),
         BILGE_ALARM_VERSION,
         IOT_DEVICE_VERSION);

    proc_leave();
    LOGI("bilgeAlarm2.ino setup() finished",0);
}



void loop()
{
    my_iot_device->loop();

    #if 1
        if (my_iot_device->getBool(ID_DEMO_MODE))
        {
            uint32_t now = millis();
            static uint32_t toggle_led = 0;
            if (now > toggle_led + 2000)
            {
                toggle_led = now;
                bool led_state = my_iot_device->getBool(ID_ONBOARD_LED);
                my_iot_device->setBool(ID_ONBOARD_LED,!led_state);  // !bilgeAlarm::m_ONBOARD_LED);
            }
        }
    #endif
}
