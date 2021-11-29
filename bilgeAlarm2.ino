
#include <myIOTDevice.h>
#include <myIOTLog.h>

#define BILGE_ALARM_VERSION "0.03"

#define ONBOARD_LED             2

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


const IOTPreference_t bilge_prefs[] =
{
    { "DISABLED",         1000,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { "BACKLIGHT_SECS",   1010,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_BACKLIGHT_SECS, 0, 255}} },
    { "ERR_RUN_TIME",     1020,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_ERR_RUN_TIME, 0, 255}} },
    { "CRIT_RUN_TIME",    1030,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_CRIT_RUN_TIME, 0, 255}} },
    { "ERR_PER_HOUR",     1040,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_ERR_PER_HOUR, 0, 255}} },
    { "ERR_PER_DAY",      1050,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_ERR_PER_DAY, 0, 255}} },
    { "EXTRA_RUN_TIME",   1060,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_EXTRA_RUN_TIME, 0, 255}} },
    { "EXTRA_RUN_MODE",   1070,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_EXTRA_RUN_MODE, 0, 1}} },
    { "END_RUN_DELAY",    1080,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_END_RUN_DELAY, 0, 255}} },
    { "RUN_EMERGENCY",    1090,    PREFERENCE_TYPE_INT,     { .int_range = { DEFAULT_RUN_EMERGENCY, 0, 255}} },
};

#define NUM_BILGE_PREFS (sizeof(bilge_prefs)/sizeof(IOTPreference_t))



class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm()
    {
        addPrefs(bilge_prefs,NUM_BILGE_PREFS);
        addTopic("LED");
        m_state_LED = "off";
    }
    ~bilgeAlarm() {}

    virtual const char *getName() override  { return "bilgeAlarm"; }
    virtual const char *getVersion() override  { return BILGE_ALARM_VERSION; }

    // In Node Red I have created a flow that consists of a switch
    // which indicates the INPUT to the topic, and OUTPUTS pubslishes
    // a retained MQQT state.

    virtual void onTopicMsg(String topic, String msg)
    {
        proc_entry();
        LOGD("bilgeAlarm::onTopicMsg(%s,%s)",topic.c_str(),msg.c_str());
        if (topic == "LED")
        {
            digitalWrite(ONBOARD_LED,msg == "on" ? 1 : 0);
            m_state_LED = msg;
        }
        proc_leave();
    }

    virtual String getTopicState(String topic) override
    {
        if (topic == "LED")
            return m_state_LED;
    }

    String m_state_LED;
};



void setup()
{
    Serial.begin(115200);
    delay(1000);
    LOGI("bilgeAlarm2.ino setup() started");

    proc_entry();

    pinMode(ONBOARD_LED,OUTPUT);
    digitalWrite(ONBOARD_LED,0);

    my_iot_device = new bilgeAlarm();
    my_iot_device->setup();

    LOGI("%s version=%s IOTVersion=%s",
         my_iot_device->getFriendlyName().c_str(),
         BILGE_ALARM_VERSION,
         IOT_DEVICE_VERSION);

    proc_leave();
    LOGI("bilgeAlarm2.ino setup() finished",0);
}


void loop()
{
    my_iot_device->loop();
}
