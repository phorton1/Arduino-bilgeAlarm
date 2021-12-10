#define INIT_SD_EARLY

#include <myIOTDevice.h>
#include <myIOTLog.h>
#include <cstdlib>

#ifdef INIT_SD_EARLY
    #include <SD.h>
#endif

#define BILGE_ALARM_VERSION "0.02"

#define ONBOARD_LED             2
#define OTHER_LED               13


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


const iotElement_t bilge_elements[] =
{
    { "DISABLED",         ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_DISABLED, 0, 1}} },
    { "BACKLIGHT_SECS",   ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_BACKLIGHT_SECS, 0, 255}} },
    { "ERR_RUN_TIME",     ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_ERR_RUN_TIME, 0, 255}} },
    { "CRIT_RUN_TIME",    ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_CRIT_RUN_TIME, 0, 255}} },
    { "ERR_PER_HOUR",     ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_ERR_PER_HOUR, 0, 255}} },
    { "ERR_PER_DAY",      ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_ERR_PER_DAY, 0, 255}} },
    { "EXTRA_RUN_TIME",   ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_EXTRA_RUN_TIME, 0, 255}} },
    { "EXTRA_RUN_MODE",   ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_EXTRA_RUN_MODE, 0, 1}} },
    { "END_RUN_DELAY",    ELEMENT_CLASS_PREF,    ELEMENT_TYPE_INT,     NULL,            { .int_range = { DEFAULT_END_RUN_DELAY, 0, 255}} },
    { "RUN_EMERGENCY",    ELEMENT_CLASS_PREF,    ELEMENT_TYPE_FLOAT,   NULL,            { .float_range = {0, -1233.456, 1233.456}} },  // int_range = { DEFAULT_RUN_EMERGENCY, 0, 255}} },
    { "ONBOARD_LED",      ELEMENT_CLASS_TOPIC,   ELEMENT_TYPE_INT,     "switch,input",  { .int_range = { 0, 0, 1}} },
    { "OTHER_LED",        ELEMENT_CLASS_TOPIC,   ELEMENT_TYPE_INT,     "switch,input",  { .int_range = { 0, 0, 1}} },

};

#define NUM_BILGE_ELEMENTS (sizeof(bilge_elements)/sizeof(iotElement_t))



class bilgeAlarm : public myIOTDevice
{
public:

    bilgeAlarm()
    {
        pinMode(ONBOARD_LED,OUTPUT);
        pinMode(OTHER_LED,OUTPUT);
        digitalWrite(ONBOARD_LED,0);
        digitalWrite(OTHER_LED,0);
        m_state_ONBOARD_LED = 0;
        m_state_OTHER_LED = 0;
        addElements(bilge_elements,NUM_BILGE_ELEMENTS);
    }
    ~bilgeAlarm() {}

    virtual const char *getName() override  { return "bilgeAlarm"; }
    virtual const char *getVersion() override  { return BILGE_ALARM_VERSION; }

    // In Node Red I have created a flow that consists of a switch
    // which indicates the INPUT to the topic, and OUTPUTS pubslishes
    // a retained MQQT state.

    virtual void onTopicMsg(String topic, String msg) override
    {
        proc_entry();
        LOGD("bilgeAlarm::onTopicMsg(%s,%s)",topic.c_str(),msg.c_str());
        if (topic == "ONBOARD_LED")
        {
            m_state_ONBOARD_LED = msg == "1" ? 1 : 0;
            digitalWrite(ONBOARD_LED,m_state_ONBOARD_LED);
        }
        else if (topic == "OTHER_LED")
        {
            m_state_OTHER_LED = msg == "1" ? 1 : 0;
            digitalWrite(OTHER_LED,m_state_OTHER_LED);
        }
        else
            myIOTDevice::onTopicMsg(topic,msg);
        proc_leave();
    }

    virtual String getTopicState(String topic) override
    {
        if (topic == "ONBOARD_LED")
            return String(m_state_ONBOARD_LED);
        if (topic == "OTHER_LED")
            return String(m_state_OTHER_LED);
        return myIOTDevice::getTopicState(topic);
    }

private:

    bool m_state_ONBOARD_LED;
    bool m_state_OTHER_LED;
};



//--------------------------------
// power monitor
//--------------------------------

#include <esp_adc_cal.h>

// #define DEBUG_POWER

#define PIN_MAIN_VOLTAGE   35       //  3.3V == 18.5V
#define PIN_MAIN_CURRENT   34       // Divide 0..5V to 0..2.5V   1024 = 0 Amps  2048=5A   0=-5A

// calibration values at 12.5V
// and approximate current consumption

#define VOLT_CALIB_OFFSET  0.00     // 0.22
#define AMP_CALIB_OFFSET   0.025     // -0.03

#define VOLTAGE_DIVIDER_RATIO    ((4640.0 + 1003.0)/1003.0)       // (R1 + R2)/R2   ~5.6, so 3.3V==18.5V
    // "4.72K" and "1K" == approx 3 ma through to ground at 12.5V
#define CURRENT_DIVIDER_RATIO      ((9410.0 + 9410.0)/9410.0)       // (R1 + R2)/R2   ~2, so 3.3V==6.6V (we need 0..5)                                       ))
    // negligble 0.263 ma current to ground

#define CIRC_BUF_SIZE   100

int volt_buf[CIRC_BUF_SIZE];
int amp_buf[CIRC_BUF_SIZE];
int volt_buf_num = 0;
int amp_buf_num = 0;

void powerTask(void *param)
{
    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);    // I believe this is vTaskDelay(1) = 10ms
        volt_buf[volt_buf_num++] = analogRead(PIN_MAIN_VOLTAGE);
        if (volt_buf_num >= CIRC_BUF_SIZE) volt_buf_num = 0;
        amp_buf[amp_buf_num++] = analogRead(PIN_MAIN_CURRENT);
        if (amp_buf_num >= CIRC_BUF_SIZE) amp_buf_num = 0;
    }
}



float getActualVolts(int *buf)
 {
    int val = 0;
    for (int i=0; i<CIRC_BUF_SIZE; i++)
    {
        val += buf[i];
    }
    val /= CIRC_BUF_SIZE;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1200, &adc_chars);
        // prh dunno what it does, but it's way better than analogRead()/4096 * 3.3V
        // The 1100 is from https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
        // The constant does not seem to do anything
    uint32_t raw_millis = esp_adc_cal_raw_to_voltage(val, &adc_chars);
    return (float) raw_millis/1000.0;
}


float getMainVoltage()
{
    float raw_volts = getActualVolts(volt_buf);
    float undivided_volts = raw_volts * VOLTAGE_DIVIDER_RATIO;
    float final_volts = undivided_volts + VOLT_CALIB_OFFSET;
    #ifdef DEBUG_POWER
        LOGD("MAIN    raw=%-1.3f   undiv=%-2.3f   final=%-2.3f",
            raw_volts,
            undivided_volts,
            final_volts);
    #endif
    return final_volts;
}

float getMainCurrent()
{
    float raw_volts = getActualVolts(amp_buf);
    float undivided_volts = raw_volts * CURRENT_DIVIDER_RATIO;
    float biased_volts = undivided_volts - 2.5;
    float current = (biased_volts/2.5) * 5.0;
    float final_current = current + AMP_CALIB_OFFSET;
    #ifdef DEBUG_POWER
        LOGD("CURRENT raw=%-1.3f   undiv=%-2.3f   biased=%-2.3f   amps=%-2.3f   final=%-2.3f",
            raw_volts,
            undivided_volts,
            biased_volts,
            current,
            final_current);
    #endif
    return final_current;
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

    #ifdef INIT_SD_EARLY
        delay(200);
        bool sd_ok = SD.begin(5);     // 5
        LOGI("sd_ok=%d",sd_ok);
    #endif

    proc_entry();

    my_iot_device = bilge_alarm = new bilgeAlarm();
    my_iot_device->setup();

    LOGI("%s version=%s IOTVersion=%s",
         my_iot_device->getFriendlyName().c_str(),
         BILGE_ALARM_VERSION,
         IOT_DEVICE_VERSION);

    LOGD("starting powerTask");
    xTaskCreate(powerTask,
        "powerTask",
        2048,
        NULL,   // param
        1,  	// priority
        NULL);  // *handle

    proc_leave();
    LOGI("bilgeAlarm2.ino setup() finished",0);
}



void loop()
{
    #if 0
        // turn the LED for between 2 and 10 seconds every
        // 30 seconds to 30 minutes
        uint32_t now = millis();
        static uint32_t last_random_time = 0;
        static uint32_t next_random_amount = 30000;
        if (now > last_random_time + next_random_amount)
        {
            bool on = bilge_alarm->m_state_LED == "on";
            uint32_t min = on ? 30*1000 : 2000;
            uint32_t max = on ? 1800*1000 : 8000;

            last_random_time = now;
            float amount = std::rand();
            amount /= RAND_MAX;
            next_random_amount = (amount * max) + min;

            String cmd("#");
            cmd += "LED=";
            cmd += on ? "off" : "on";
            bilge_alarm->handleCommand(cmd);

            LOGD("RANDOM %s amount=%0.3f min=%d max=%d next=%d",cmd.c_str(),
                 amount,min,max,next_random_amount);
        }
    #endif

    my_iot_device->loop();

    static bool led_state = 0;
    static uint32_t toggle_led = 0;
    uint32_t now = millis();

    if (now > toggle_led + 1000)
    {
        toggle_led = now;
        led_state = !led_state;
        digitalWrite(ONBOARD_LED,led_state);
    }

    if (my_iot_device->getConnectStatus() == IOT_CONNECT_STA)
    {
        static uint32_t check_time =0;
        if (now > check_time + 1000)
        {
            check_time = now;
            float main_volts = getMainVoltage();
            float main_amps = getMainCurrent();
            String msg = "{\"power_volts\":";
            msg += String(main_volts,2);
            msg += ",\"power_amps\":";
            msg += String(main_amps,3);
            msg += "}";
            my_iot_device->wsBroadcast(msg.c_str());

            #if 0
                LOGD("POWER %0.2fV %0.3fA",main_volts,main_amps);
            #endif
        }
    }

}
