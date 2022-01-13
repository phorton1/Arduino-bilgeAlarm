# 1 "C:\\src\\Arduino\\bilgeAlarm\\bilgeAlarm.ino"
//-----------------------------------
// bilgeAlarm.ino
//-----------------------------------
# 5 "C:\\src\\Arduino\\bilgeAlarm\\bilgeAlarm.ino" 2
# 6 "C:\\src\\Arduino\\bilgeAlarm\\bilgeAlarm.ino" 2


#define INIT_SD_EARLY 



//--------------------------------
// main
//--------------------------------


void setup()
{
    Serial.begin(115200);
    delay(1000);

    bilgeAlarm::setDeviceType("bilgeAlarm");
    bilgeAlarm::setDeviceVersion("b0.05");

    // init the SD Card in early derived device
    // due to it's wonky SPI behavior, and so that
    // logfile can begin immediately.



        bool sd_ok = bilgeAlarm::initSDCard();



    LOGU("");
    LOGU("");
    LOGU("bilgeAlarm.ino setup() started on core(%d)",xPortGetCoreID());



        LOGD("sd_ok=%d",sd_ok);



    bilge_alarm = new bilgeAlarm();
    bilge_alarm->setup();

    LOGU("bilgeAlarm.ino setup() finished",0);
}



void loop()
{
    bilge_alarm->loop();
# 70 "C:\\src\\Arduino\\bilgeAlarm\\bilgeAlarm.ino"
}
