#include <myDebug.h>
#include <myIOTDevice.h>

const IOTPreference_t bilge_prefs[] =
{
    { "NUM_RETRIES",    100,    PREFERENCE_TYPE_INT,    { .int_range = { 3, 0, 5}} },
    { "NUM_PER_HOUR",   110,    PREFERENCE_TYPE_INT,    { .int_range = { 6, 1, 20}} },
};

#define NUM_BILGE_PREFS (sizeof(bilge_prefs)/sizeof(IOTPreference_t))

class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm()
    {
        addPrefs(bilge_prefs,NUM_BILGE_PREFS);
    }
    ~bilgeAlarm() {}

    virtual const char *getName() override  { return "bilgeAlarm"; }

};





void setup()
{
    Serial.begin(115200);
    delay(1000);
    display(0,"bilgeAlarm2.ino setup() started",0);
    proc_entry();

    my_iot_device = new bilgeAlarm();

    String s = my_iot_device->getPreferencesList();
    Serial.print("bilgePrefs=");
    Serial.println(s);


    my_iot_device->setup();
    proc_leave();
    display(0,"bilgeAlarm2.ino setup() completed",0);
}


void loop()
{
    my_iot_device->loop();
}
