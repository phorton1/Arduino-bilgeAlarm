//-----------------------------------------
// uiScreen.cpp
//-----------------------------------------
//
// SEE IMPORTANT NOTE in myIOTTypes.h regarding the fact
// that certain tasks must run on ESP32_CORE_ARDUINO == 1
// as the lcd only works from the same core it was inited on.
//
//------------------------
// UI BUTTONS
//------------------------
//
// Errors take over the whole button scheme
//     - any button to suppress alarm
//     - any button to clear alarm
// Otherwise, if MENU_TIME is set, and no buttons are pressed
//     after MENU_TIME seconds the system returns to the
//     MAIN_MODE and MAIN_SCREEN.
//
// A long click of the LEFT button while not on the MAIN_SCREEN
//     returns to the MAIN_SCREEN (and MAIN_MODE)
// A long click of the LEFT button in MAIN_MODE goes to CONFIG_MODE
// A long click of the MIDDLE button in MAIN_MODE goes to DEVICE_MODE
//
// A click of the MIDDLE button from the MAIN_SCREEN goes to HISTORY mode
//     In history mode the MIDDLE button (continues to) increments screens
//     wheaas the RIGHT button decrements screens (both with wrapping)
//     and the LEFT button returns to MAIN mode
// Otherwise in MAIN, CONFIG, and DEVICE modes (generally)
//     a click of LEFT button cycles through screen within the mode
//     The RIGHT button is used to perform commands
//     The MIDDLE and RIGHT buttons are decrement and increment for values
// In commands that require a confirm,
//     the RIGHT button is CONFIRM, and the MIDDLE button is CANCEL


#include "uiScreen.h"
#include "uiButtons.h"
#include <myIOTLog.h>
#include <LiquidCrystal_I2C.h>

#define MIN_BACKLIGHT       15
#define MIN_MENUTIMEOUT     15

#define DEBUG_SCREEN  1


#if DEBUG_SCREEN
    #define DBG_SCREEN(...)     LOGD(__VA_ARGS__)
#else
    #define DBG_SCREEN(...)
#endif



#define REFRESH_MILLIS            30

#define LCD_LINE_LEN              16
#define LCD_BUF_LEN               32    // for safety

#define AUTO_ERROR_DURATION       2000
    // rotate error screens
#define AUTO_MAIN_DURATION        3000
    // keep the main screen on a little longer
#define ALARM_CLEARED_DURATION    2000


#define MENU_MODE_MAIN      0     // main mode - main screen and commands
#define MENU_MODE_HISTORY   1     // stats mode - cycle through history if any
#define MENU_MODE_CONFIG    2     // config mode - modify config options


#define SCREEN_ERROR            0
#define SCREEN_PRESS_TO_QUIET   1
#define SCREEN_PRESS_TO_CLEAR   2
#define SCREEN_ALARM_CLEARED    3

#define SCREEN_MAIN             4
#define SCREEN_WIFI             5
#define SCREEN_POWER            6
#define SCREEN_RELAY            7
#define SCREEN_SELFTEST         8
#define SCREEN_CLEAR_HISTORY    9
#define SCREEN_REBOOT           10
#define SCREEN_FACTORY_RESET    11
#define LAST_MAIN_SCREEN        11

#define SCREEN_CONFIRM          12

#define SCREEN_HISTORY_BASE     13
#define SCREEN_HISTORY          14

#define SCREEN_CONFIG_BASE      15
#define SCREEN_CONFIG           16


const char *screens[] = {

    "%s",                   "%s",                   // 0
    "PRESS ANY KEY TO",     "SILENCE ALARM",        // 1
    "PRESS ANY KEY TO",     "CLEAR ALARM",          // 2
    "ALARM",                "CLEARED",              // 3

    "HOUR %-3d DAY %-3d",   "%-8s%8s",              // 4
    "%-16s",                "%16s",                 // 5
    "MAIN       %4sV",      "CPU        %4sV",      // 6
    "PRIMARY PUMP",         "RELAY        %-3S",    // 7
    "SELF TEST",            "         PERFORM",     // 8
    "CLEAR HISTORY",        "         PERFORM",     // 9
    "REBOOT",               "         PERFORM",     // 10
    "FACTORY RESET",        "         PERFORM",     // 11

    "CONFIRM",              "%S",                   // 12

    "HISTORY %8d",          "BACK  NEXT  PREV",     // 13
    "%s %s",                "%-12s %3d",            // 14

    "CONFIG SETTINGS",      "NEXT",                 // 15
    "%s",                   "%16s",                 // 16
};


// things that show up in CONFIG_MODE

static valueIdType config_mode_ids[] = {
#if DEMO_MODE
    ID_DEMO_MODE,
#endif

    ID_DISABLED,
    ID_ERR_RUN_TIME,
    ID_CRIT_RUN_TIME,
    ID_ERR_PER_HOUR,
    ID_ERR_PER_DAY,
    ID_RUN_EMERGENCY,
    ID_EXTRA_RUN_TIME,
    ID_EXTRA_RUN_MODE,
    ID_EXTRA_RUN_DELAY,

    ID_BACKLIGHT_SECS,      // from bilgeAlarm
    ID_MENU_SECS,           // from bilgeAlarm
    ID_LED_BRIGHT,
    ID_EXT_LED_BRIGHT,

    ID_DEBUG_LEVEL,
    ID_LOG_LEVEL,

    ID_DEVICE_NAME,         // strings are forced to read-only
    ID_DEVICE_TYPE,
    ID_DEVICE_VERSION,
    ID_DEVICE_UUID,

    ID_CALIB_12V,
    ID_CALIB_5V,
    ID_SENSE_MILLIS,
    ID_PUMP_DEBOUNCE,
    ID_RELAY_DEBOUNCE,
    ID_SW_THRESHOLD,

};



uiScreen *ui_screen = NULL;
uiButtons *ui_buttons = NULL;
LiquidCrystal_I2C lcd(0x27,LCD_LINE_LEN,2);   // 20,4);
    // set the LCD address to 0x27 for a 16 chars and 2 line display



void uiScreen::initValueMembers()
{
    m_value = 0;    // pointer to myIOTValue
    m_value_id = 0;
    m_value_type = 0;
    m_value_style = 0;
    m_value_min = 0;
    m_value_max = 0;
    m_cur_value = 0;    // the actual value
}


#include <rom/rtc.h>

static const char *bootReason()
{
    switch (rtc_get_reset_reason(0))
    {
        case NO_MEAN                : return "NO_MEAN        ";
        case POWERON_RESET          : return "POWERON_RESET  ";    /**<1, Vbat power on reset*/
        case SW_RESET               : return "SW_RESET       ";    /**<3, Software reset digital core*/
        case OWDT_RESET             : return "OWDT_RESET     ";    /**<4, Legacy watch dog reset digital core*/
        case DEEPSLEEP_RESET        : return "DEEPSLEEP_RESET";    /**<3, Deep Sleep reset digital core*/
        case SDIO_RESET             : return "SDIO_RESET     ";    /**<6, Reset by SLC module, reset digital core*/
        case TG0WDT_SYS_RESET       : return "TG0WDT_SYS_RESE";    /**<7, Timer Group0 Watch dog reset digital core*/
        case TG1WDT_SYS_RESET       : return "TG1WDT_SYS_RESE";    /**<8, Timer Group1 Watch dog reset digital core*/
        case RTCWDT_SYS_RESET       : return "RTCWDT_SYS_RESE";    /**<9, RTC Watch dog Reset digital core*/
        case INTRUSION_RESET        : return "INTRUSION_RESET";    /**<10, Instrusion tested to reset CPU*/
        case TGWDT_CPU_RESET        : return "TGWDT_CPU_RESET";    /**<11, Time Group reset CPU*/
        case SW_CPU_RESET           : return "SW_CPU_RESET   ";    /**<12, Software reset CPU*/
        case RTCWDT_CPU_RESET       : return "RTCWDT_CPU_RESE";    /**<13, RTC Watch dog Reset CPU*/
        case EXT_CPU_RESET          : return "EXT_CPU_RESET  ";    /**<14, for APP CPU, reseted by PRO CPU*/
        case RTCWDT_BROWN_OUT_RESET : return "RTCWDT_BROWN_OU";    /**<15, Reset when the vdd voltage is not stable*/
        case RTCWDT_RTC_RESET       : return "RTCWDT_RTC_RESE";    /**<16, RTC Watch dog reset digital core and rtc module*/
        default                     : return "UNKNOWN_REASON ";
    }
}



uiScreen::uiScreen(int *button_pins)
{
    m_started = false;
    m_menu_mode = MENU_MODE_MAIN;
    m_screen_num = -1;
    m_prev_screen = 0;

    m_last_time = 0;
    m_last_screen_time = 0;
    m_last_state = 0;
    m_last_since = 0;

    m_backlight = 0;
    m_activity_time = 0;

    m_hist_num = 0;
    m_hist_iter = 0;
    m_hist_ptr = NULL;

    m_value_num = 0;
    m_value_count = 0;
    m_value_ids = NULL;
    initValueMembers();

    ui_screen = this;
    ui_buttons = new uiButtons(button_pins);

    lcd.init();                      // initialize the lcd
    lcd.backlight();

    #if 0
        lcd.setCursor(0,0);
        lcd.print(bootReason());
        delay(2000);
    #endif

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(0,bilgeAlarm::getDeviceType());
    lcd.setCursor(0,1);
    lcd.print(bilgeAlarm::getVersion());
    backlight(1);
}


//-----------------------------------
// utilities
//-----------------------------------

static const char *stateName(uint32_t state)
{
    //                                             "                "
    if (state & STATE_PUMP2_ON)             return "EMERG PUMP IS ON";
    if (state & STATE_EMERGENCY)            return "EMERG PUMP RAN";
    if (state & STATE_CRITICAL_TOO_LONG)    return "RUN WAY TOO LONG";
    if (state & STATE_TOO_LONG)             return "RUN TOO LONG";
    if (state & STATE_TOO_OFTEN_DAY)        return "TOO OFTEN DAY";
    if (state & STATE_TOO_OFTEN_HOUR)       return "TOO OFTEN HOUR";
    if (state & STATE_RELAY_EMERGENCY)      return "RELAY_EMERGENCY";
    if (state & STATE_RELAY_ON)             return "RELAY_ON";
    if (state & STATE_RELAY_FORCED)         return "FORCE RELAY ON";
    if (state & STATE_PUMP1_ON)             return "PUMP_ON";
    return "NONE";
}


static const char *alarmStateName(uint32_t alarm_state)
{
    if (alarm_state & ALARM_STATE_EMERGENCY ) return "EMERGENCY";
    if (alarm_state & ALARM_STATE_SUPPRESSED) return "ALARM SUPRESSED";
    if (alarm_state & ALARM_STATE_CRITICAL)   return "CRITICAL";
    if (alarm_state & ALARM_STATE_ERROR)      return "ERROR";
    return "NONE";
}



static const char *off_on(bool b)
{
    return b ? "on" : "off";
}


void uiScreen::backlight(int val)
{
    m_backlight = val;
    m_activity_time = millis();
    if (val)
        lcd.backlight();
    else
        lcd.noBacklight();
}


static void print_lcd(int lcd_line, int screen_line, ...)
{
    va_list var;
    va_start(var, screen_line);
    char buffer[LCD_BUF_LEN];

    vsnprintf(buffer,LCD_BUF_LEN,screens[screen_line],var);
    int len = strlen(buffer);
    if (len > LCD_LINE_LEN)
        len = LCD_LINE_LEN;
    while (len < LCD_LINE_LEN)
    {
        buffer[len++] = ' ';
    }
    buffer[len] = 0;
    lcd.setCursor(0,lcd_line);
    lcd.print(buffer);
}



//-----------------------------------
// loop()
//-----------------------------------

void uiScreen::loop()
{
    if (!m_started)
    {
        m_started = true;

        lcd.clear();
        char buf[LCD_BUF_LEN];
        int boot_count = bilgeAlarm::getBootCount();
        int reset_count = bilge_alarm->getInt(ID_RESET_COUNT);
        sprintf(buf,"RESET_COUNT %4d",reset_count);
        lcd.setCursor(0,0);
        lcd.print(buf);
        sprintf(buf,"BOOT_COUNT %5d",boot_count);
        lcd.setCursor(0,1);
        lcd.print(buf);

        vTaskDelay(2000/portTICK_PERIOD_MS);

        setScreen(SCREEN_MAIN);
    }

    if (!m_backlight && !bilge_alarm->getInt(ID_BACKLIGHT_SECS))
        backlight(1);

    uint32_t now = millis();
    if (now > m_last_time + REFRESH_MILLIS)
    {
        m_last_time = now;

        uint32_t state = bilgeAlarm::getState();
        uint32_t alarm_state = bilgeAlarm::getAlarmState();

        // turn the backlight on or off

        if (alarm_state)
        {
            if (m_backlight != 2)
                backlight(2);
        }
        else if (m_backlight == 2)
        {
            backlight(1);
        }
        else if (m_backlight == 1)
        {
            int secs = bilge_alarm->getInt(ID_BACKLIGHT_SECS);
            if (secs && secs < MIN_BACKLIGHT) secs = MIN_BACKLIGHT;
            if (secs && now > m_activity_time + secs * 1000)
                backlight(0);
        }

        // return to main screen

        int menu_secs = bilge_alarm->getInt(ID_MENU_SECS);
        if (menu_secs && !alarm_state &&
            m_screen_num != SCREEN_MAIN &&
            now > m_activity_time + menu_secs * 1000)
        {
            setScreen(SCREEN_MAIN);
        }

        // handle rotating screens
        // or pop into error mode

        switch (m_screen_num)
        {
            case SCREEN_MAIN:
                if (alarm_state &&
                    now > m_last_screen_time + AUTO_MAIN_DURATION)
                {
                    setScreen(SCREEN_ERROR);
                }
                else
                {
                    uint32_t last = bilge_alarm->getInt(ID_SINCE_LAST_RUN);
                    if (last)
                    {
                        uint32_t since = time(NULL) - last;
                        if (since != m_last_since)
                        {
                            m_last_since = since;
                            setScreen(m_screen_num);
                        }
                    }
                }
                break;

            case SCREEN_ERROR:
                if (alarm_state)
                {
                    if (now > m_last_screen_time + AUTO_ERROR_DURATION)
                    {
                        if (alarm_state & ALARM_STATE_SUPPRESSED)
                            setScreen(SCREEN_PRESS_TO_CLEAR);
                        else
                            setScreen(SCREEN_PRESS_TO_QUIET);
                    }
                }
                else // if someone else clears the error (including auto-self-clearing)
                    setScreen(SCREEN_MAIN);
                break;

            case SCREEN_PRESS_TO_CLEAR:
            case SCREEN_PRESS_TO_QUIET:
                if (alarm_state)
                {
                    if (now > m_last_screen_time + AUTO_ERROR_DURATION)
                    {
                        // if the emergency pump is on, just rotate
                        // between the error and the "press to silence"
                        // buttons ... otherwise, go to the stats screen
                        // in between.

                        if (alarm_state && ALARM_STATE_EMERGENCY &&
                            !(alarm_state && ALARM_STATE_SUPPRESSED))
                        {
                            setScreen(SCREEN_ERROR);
                        }
                        else
                        {
                            setScreen(SCREEN_MAIN);
                        }
                    }
                }
                else    // if someone else clears the error (including auto-self-clearing)
                    setScreen(SCREEN_MAIN);
                break;

            case SCREEN_ALARM_CLEARED:
                if (now > m_last_screen_time + ALARM_CLEARED_DURATION)
                    setScreen(SCREEN_MAIN);
                break;

            // Refresh the FORCE_RELAY "command" state if somebody else changes it

            // case SCREEN_RELAY:
            //     if ((state & STATE_RELAY_FORCED) != (m_last_state & STATE_RELAY_FORCED))
            //         setScreen(m_screen_num);
            //
            //     // fall through to default case in case we just entered an error condition
            //     // while in the FORCE_RELAY command ... this is special code because we are
            //     // only dealing with one bit of the state.  For preferences, we will use
            //     // the onValueChange stuff.

            default:
                if (alarm_state)
                {
                    if (m_screen_num > SCREEN_MAIN)
                        setScreen(SCREEN_ERROR);
                }
                else if (state != m_last_state)
                    setScreen(m_screen_num);
                break;
        }

        // check the buttons

        ui_buttons->loop();

        // save the state for comparison next time through

        m_last_state = state;
    }

}   // uiScreen::loop()



//-----------------------------------
// Values
//-----------------------------------

void uiScreen::onValueChanged(const myIOTValue *value, valueStore from)
{
    if (!m_started)
        return;

    const char *id = value->getId();

    // DBG_SCREEN("uiScreen onValueChanged(%s) from(%d) on core(%d)",id,from,xPortGetCoreID());

    if (!strcmp(id,ID_REBOOT))
    {
        lcd.setCursor(0,0);
        lcd.print("   REBOOTING!   ");
        lcd.setCursor(0,1);
        lcd.print("                ");
    }

    // reset activity time on any change to
    // menu or backlight timeout parameters

    if (!strcmp(id,ID_MENU_SECS) ||
        !strcmp(id,ID_BACKLIGHT_SECS))
    {
        m_activity_time = millis();
    }

    // check if the current main screen needs redraw
    // we redraw all the main screens if the state
    // or anything shown on the main screen changes.
    // this also catches the FORCE_RELAY at the minor
    // cost of updating any other main creens they happen
    // to be sitting on.

    if (m_menu_mode == MENU_MODE_MAIN)
    {
       if (!strcmp(id,ID_STATE) ||
           !strcmp(id,ID_SINCE_LAST_RUN) ||
           !strcmp(id,ID_NUM_LAST_DAY) ||
           !strcmp(id,ID_NUM_LAST_HOUR) ||
           !strcmp(id,ID_NUM_LAST_WEEK))
        {
            setScreen(m_screen_num);
            return;
        }
        if (m_screen_num == SCREEN_POWER && (
            !strcmp(id,ID_POWER_12V) ||
            !strcmp(id,ID_POWER_5V)))
        {
            setScreen(m_screen_num);
            return;
        }
    }

    // othersise, ignore changes coming from
    // our own values being changed ...

    if (from == VALUE_STORE_PROG)
        return;

    // but redraw if the value being edited changes

    bool needs_draw = false;
    if (m_value_count)
    {
        for (int i=0; i<m_value_count; i++)
        {
            if (!strcmp(id,m_value_ids[i]))
            {
                needs_draw = 1;
                break;
            }
        }
    }
    if (needs_draw)
        setScreen(m_screen_num);

}   // uiScreen::onValueChanged()


bool uiScreen::handleValue(int button_num, int event_type)
{
    DBG_SCREEN("handleValue(%d/%d) button(%d:%d) mode(%d) screen(%d)",m_value_num,m_value_count,button_num,event_type,m_menu_mode,m_screen_num);
    if (!m_value || !m_value_type)
    {
        LOGE("implementation error - value members not initialized");
        return true;
    }
    if (m_value_style & VALUE_STYLE_READONLY)
        return false;

    if (event_type == BUTTON_TYPE_CLICK)    // RELEASED
    {
        DBG_SCREEN("commit_value(%d)",m_cur_value);
        if (m_value_type == VALUE_TYPE_ENUM)
            m_value->setEnum(m_cur_value);
        else if (m_value_type == VALUE_TYPE_BOOL)
            m_value->setBool(m_cur_value);
        else if (m_value_type == VALUE_TYPE_INT)
            m_value->setInt(m_cur_value);
        return true;
    }

    // allow either button to toggle booleans
    // allow enums to wrap

    int start_value = m_cur_value;

    if (m_value_type == VALUE_TYPE_BOOL)
    {
        DBG_SCREEN("toggle_bool(%d)",m_cur_value);
        m_cur_value = !m_cur_value;
    }
    else if (button_num == 2)
    {
        DBG_SCREEN("inc_value(%d) min(%d) max(%d)",m_cur_value,m_value_min,m_value_max);
        if (m_cur_value < m_value_max)
            m_cur_value++;
        else if (m_value_type == VALUE_TYPE_ENUM)
            m_cur_value = 0;

        // handle ZERO_OFF_GAP
        if (m_cur_value < m_value_min)
            m_cur_value = m_value_min;
    }
    else if (button_num == 1)
    {
        DBG_SCREEN("dec_value(%d) min(%d) max(%d)",m_cur_value,m_value_min,m_value_max);
        if (m_cur_value > m_value_min)
            m_cur_value--;
        else if (m_value_style & VALUE_STYLE_OFF_ZERO)
            m_cur_value = 0;
        else if (m_value_type == VALUE_TYPE_ENUM)
            m_cur_value = m_value_max;
        print_lcd(1,m_screen_num * 2 + 1,m_value->getIntAsString(m_cur_value).c_str());
    }

    if (m_cur_value != start_value)
        print_lcd(1,m_screen_num * 2 + 1,m_value->getIntAsString(m_cur_value).c_str());

    return false;
}



//-----------------------------------
// setScreen()
//-----------------------------------

void uiScreen::setScreen(int screen_num)
{
    // we always init current value members on a new screen

    initValueMembers();

    // set m_last_screen_time to the last time m_screen_num changed ...
    // thia ia used for auto-rotations

    if (m_screen_num != screen_num)
    {
        m_prev_screen = m_screen_num;
        DBG_SCREEN("setScreen(%d) mode(%d) value(%d/%d)",screen_num,m_menu_mode,m_value_num,m_value_count);
        m_last_screen_time = millis();
        m_screen_num = screen_num;
    }

    // handle MODE and semi-mode changes

    if (screen_num >= SCREEN_CONFIG_BASE)
    {
        m_menu_mode = MENU_MODE_CONFIG;
        m_value_ids = config_mode_ids;
        m_value_count = sizeof(config_mode_ids)/sizeof(valueIdType *);
        if (screen_num == SCREEN_CONFIG_BASE)
        {
            m_value_num = 0;
            ui_buttons->setRepeatMask(0);
        }
        else
            ui_buttons->setRepeatMask(6);
    }
    else if (screen_num >= SCREEN_HISTORY_BASE)
    {
        m_menu_mode = MENU_MODE_HISTORY;
        if (screen_num == SCREEN_HISTORY)
            ui_buttons->setRepeatMask(6);
        else
            ui_buttons->setRepeatMask(0);
    }
    else
    {
        m_menu_mode = MENU_MODE_MAIN;
        ui_buttons->setRepeatMask(0);
    }

    // setup for screen draws

    uint32_t state = bilgeAlarm::getState();
    uint32_t alarm_state = bilgeAlarm::getAlarmState();

    if (alarm_state)
        ui_buttons->setRepeatMask(0);

    int n0 = m_screen_num * 2;
    int n1 = m_screen_num * 2 + 1;
        // the line numbers for the screen

    // do screen draws

    switch (m_screen_num)
    {
        // screens with no params

        case SCREEN_CONFIG_BASE:
        case SCREEN_SELFTEST:
        case SCREEN_CLEAR_HISTORY:
        case SCREEN_REBOOT:
        case SCREEN_FACTORY_RESET:
        case SCREEN_ALARM_CLEARED:
        case SCREEN_PRESS_TO_CLEAR:
        case SCREEN_PRESS_TO_QUIET:
            print_lcd(0,n0);
            print_lcd(1,n1);
            break;

        case SCREEN_ERROR:
            print_lcd(0,n0,alarmStateName(bilgeAlarm::getAlarmState()));
            print_lcd(1,n1,stateName(bilgeAlarm::getState()));
            break;

        case SCREEN_MAIN :
        {
            int hour_count = bilge_alarm->getInt(ID_NUM_LAST_HOUR);
            int day_count = bilge_alarm->getInt(ID_NUM_LAST_DAY);
            int week_count = bilge_alarm->getInt(ID_NUM_LAST_WEEK);

            char buf[LCD_BUF_LEN] = "";
            char buf2[LCD_BUF_LEN] = "";

            if (state & STATE_RELAY_EMERGENCY)
                strcpy(buf,"ERELAY!");
            else if (state & STATE_PUMP2_ON)
                strcpy(buf,"EMERG!!");
            else if (state & STATE_RELAY_FORCED)
                strcpy(buf,"FORCE");
            else if (state & STATE_RELAY_ON)
                strcpy(buf,"RELAY");
            else if (state & STATE_PUMP1_ON)
                strcpy(buf,"PUMP ON");
            else
                sprintf(buf,"WEEK %-3d",week_count);

            m_last_since = 0;
            uint32_t last = bilge_alarm->getInt(ID_SINCE_LAST_RUN);   // time of last run as integer
            if (last)
            {
                m_last_since = time(NULL) - last;

                // we start showing just seconds,
                // then minutes:seconds
                // then hours:minutes:seconds
                // then if hours gets above 99, we switch to hours:minues
                // they are aligned under the "DAY" if possible

                int hours = m_last_since / (60 * 60);
                int mins  = (m_last_since % (60 * 60)) / 60;
                int secs  = (m_last_since % 60);

                if (hours > 99)
                    sprintf(buf2,"%d:%02d",hours,mins);
                else if (hours)
                    sprintf(buf2,"%d:%02d:%02d",hours,mins,secs);
                else if (mins)
                    sprintf(buf2,"%02d:%02d",mins,secs);
                else
                    sprintf(buf2,"%d",secs);

                while (strlen(buf2)<7)
                    strcat(buf2," ");
            }

            print_lcd(0,n0,hour_count,day_count);
            print_lcd(1,n1,buf,buf2);
            break;
        }

        // COMMANDS

        case SCREEN_WIFI:
        {
            iotConnectStatus_t mode = bilge_alarm->getConnectStatus();
            const char *mode_str =
                mode == WIFI_MODE_AP ? "WIFI_AP" :
                mode == WIFI_MODE_STA ? "WIFI_STA" :
                mode == WIFI_MODE_APSTA ? "WIFI_AP_STA" : "NO_WIFI";
            print_lcd(0,n0,mode_str);
            print_lcd(1,n1,bilge_alarm->getString(ID_DEVICE_IP).c_str());
            break;
        }

        case SCREEN_POWER:
        {
            char buf[LCD_BUF_LEN] = "";
            sprintf(buf,"%0.1f",bilge_alarm->getFloat(ID_POWER_12V));
            print_lcd(0,n0,buf);
            sprintf(buf,"%0.1f",bilge_alarm->getFloat(ID_POWER_5V));
            print_lcd(1,n1,buf);
            break;
        }

        case SCREEN_RELAY:
            print_lcd(0,n0);
            print_lcd(1,n1,off_on(state & STATE_RELAY_FORCED));
            break;

        case SCREEN_CONFIRM:
        {
            int prev_line = m_prev_screen * 2;
            print_lcd(0,n0);
            print_lcd(1,n1,screens[prev_line]);
            break;
        }

        // HISTORY

        case SCREEN_HISTORY_BASE:
            m_hist_num = 0;
            m_hist_iter = 0;
            ba_history.initIterator(&m_hist_iter);
            print_lcd(0,n0,ba_history.countRuns(COUNT_ALL));
            print_lcd(1,n1);
            break;
        case SCREEN_HISTORY:
            if (m_hist_ptr)
            {
                // prioritized string
                const char *buf = "secs";
                uint16_t flags = m_hist_ptr->flags;
                if (flags & STATE_EMERGENCY)                buf = "EMERGENCY";
                else if (flags & STATE_CRITICAL_TOO_LONG)   buf = "CRIT LONG";
                else if (flags & STATE_TOO_LONG)            buf = "ERR LONG";
                else if (flags & STATE_TOO_OFTEN_DAY)       buf = "ERR NUM/DAY";
                else if (flags & STATE_TOO_OFTEN_HOUR)      buf = "ERR NUM/HOUR";

                char buf1[LCD_BUF_LEN] = "";
                char buf2[LCD_LINE_LEN] = "";

                struct tm *ts = localtime(&m_hist_ptr->tm);
                sprintf(buf1,"%d %02d-%02d",m_hist_num,ts->tm_mon+1,ts->tm_mday);
                bool full_time = LCD_LINE_LEN - strlen(buf1) - 1 >= 8;
                if (full_time)
                    sprintf(buf2,"%02d:%02d:%02d",ts->tm_hour,ts->tm_min,ts->tm_sec);
                else
                    sprintf(buf2,"%02d:%02d",ts->tm_hour,ts->tm_min);

                print_lcd(0,n0,buf1,buf2);
                print_lcd(1,n1,buf,m_hist_ptr->dur);
            }
            break;


        // DEFAULT (device or config mode) landing on a new value

        default:
        {
            m_value_id = m_value_ids[m_value_num];
            m_value = bilge_alarm->findValueById(m_value_id);
            if (!m_value)
            {
                LOGE("setScreen(%d) value(%d/%d) could not findValue(%s)",m_screen_num,m_value_num,m_value_count,m_value_id);
                return;
            }

            m_value_type = m_value->getType();
            m_value_style = m_value->getStyle();

            // get the (integer) value or set the readonly bit on types we
            // don't know how to edit

            if (m_value_type == VALUE_TYPE_BOOL)
                m_cur_value = m_value->getBool();
             else if (m_value_type == VALUE_TYPE_INT)
                m_cur_value = m_value->getInt();
            else if (m_value_type == VALUE_TYPE_ENUM)
                m_cur_value = m_value->getEnum();
            else
                m_value_style |= VALUE_STYLE_READONLY;

            if (!(m_value_style & VALUE_STYLE_READONLY) &&
                !m_value->getIntRange(&m_value_min,&m_value_max))
            {
                LOGE("setScreen(%d) value(%d/%d) could not get min/max for value(%s)",m_screen_num,m_value_num,m_value_count,m_value_id);
                return;
            }

            String value = m_value->getAsString();
            print_lcd(0,n0,m_value_id);
            print_lcd(1,n1,value.c_str());
            break;
        }
    }

}   // setScreen()



//-----------------------------------
// onButton
//-----------------------------------

bool uiScreen::onButton(int button_num, int event_type)
    // called from uiButtons::loop()
{
    DBG_SCREEN("uiScreen::onButton(%d,%d) mode(%d) screen(%d) value(%d/%d)",button_num,event_type,m_menu_mode,m_screen_num,m_value_num,m_value_count);

    uint32_t state = bilgeAlarm::getState();
    uint32_t alarm_state = bilgeAlarm::getAlarmState();

    // eat the keystroke to turn on backlight

    if (!m_backlight)
    {
        backlight(1);
        return true;
    }

    // any key suppresses, then clears any alarms

    else if (alarm_state)
    {
        if (alarm_state & ALARM_STATE_SUPPRESSED)
        {
            bilgeAlarm::clearError();
            setScreen(SCREEN_ALARM_CLEARED);
        }
        else
        {
            bilgeAlarm::suppressAlarm();
            setScreen(SCREEN_PRESS_TO_CLEAR);
        }
        m_activity_time = millis();
        return true;
    }

    // any keystroke keeps tha backlight alive

    m_activity_time = millis();

    //---------------------------
    // MAIN_MODE
    //---------------------------

    if (m_menu_mode == MENU_MODE_MAIN)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_CONFIG_BASE);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                if (m_screen_num == LAST_MAIN_SCREEN)
                    setScreen(SCREEN_MAIN);
                else
                    setScreen(m_screen_num+1);
                return true;
            }
        }
        else
        {
            if (button_num == 1)
            {
                if (event_type == BUTTON_TYPE_CLICK)
                {
                    if (m_screen_num == SCREEN_CONFIRM)
                        setScreen(m_prev_screen);
                    else
                        setScreen(SCREEN_HISTORY_BASE);
                    return true;
                }
            }
            else if (button_num == 2)
            {
                switch (m_screen_num)
                {
                    case SCREEN_RELAY:
                    {
                        bool relay_on = state & STATE_RELAY_FORCED;
                        bilge_alarm->setBool(ID_FORCE_RELAY,!relay_on);
                        setScreen(m_screen_num);
                        break;
                    }
                    case SCREEN_SELFTEST:
                        bilgeAlarm::selfTest();
                        break;
                    case SCREEN_CLEAR_HISTORY:
                    case SCREEN_REBOOT :
                    case SCREEN_FACTORY_RESET:
                        setScreen(SCREEN_CONFIRM);
                        break;
                    case SCREEN_CONFIRM:
                        if (m_prev_screen == SCREEN_CLEAR_HISTORY)
                             bilgeAlarm::clearHistory();
                        else if (m_prev_screen == SCREEN_REBOOT)
                            bilgeAlarm::reboot();
                        else if (m_prev_screen == SCREEN_FACTORY_RESET)
                            bilgeAlarm::factoryReset();
                        setScreen(SCREEN_MAIN);
                        break;
                }
                return true;
            }
        }
    }

    //-----------------------------------
    // HISTORY_MODE
    //-----------------------------------

    else if (m_menu_mode == MENU_MODE_HISTORY)
    {
        if (button_num == 0 )
        {
            setScreen(SCREEN_MAIN);
        }
        else if (button_num == 1)
        {
            m_hist_ptr = ba_history.getNext(&m_hist_iter);
            if (m_hist_ptr)
            {
                m_hist_num++;
                setScreen(SCREEN_HISTORY);
            }
            else
            {
                setScreen(SCREEN_HISTORY_BASE);
            }
        }
        else if (button_num == 2)
        {
            if (m_screen_num == SCREEN_HISTORY_BASE)
            {
                m_hist_num = 0;
                m_hist_iter = 0;
                int iter;
                ba_history.initIterator(&iter);
                const runHistory_t *ptr = ba_history.getNext(&iter);
                while (ptr)
                {
                    m_hist_num++;
                    m_hist_ptr = ptr;
                    m_hist_iter = iter;
                    ptr = ba_history.getNext(&iter);
                }

                if (m_hist_num)
                    setScreen(SCREEN_HISTORY);
                else
                    setScreen(SCREEN_HISTORY_BASE);
            }
            else
            {
                m_hist_ptr = ba_history.getPrev(&m_hist_iter);
                if (m_hist_ptr)
                {
                    m_hist_num--;
                    setScreen(SCREEN_HISTORY);
                }
                else
                {
                    setScreen(SCREEN_HISTORY_BASE);
                }
            }
        }
        return true;
    }

    //---------------------------
    // CONFIG_MODE
    //---------------------------

    else // if (m_menu_mode == MENU_MODE_CONFIG)
    {
        if (m_screen_num == SCREEN_CONFIG_BASE &&
            event_type == BUTTON_TYPE_LONG_CLICK)
        {
            setScreen(SCREEN_MAIN);
            return true;
        }
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_MAIN);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                if (m_screen_num == SCREEN_CONFIG_BASE)
                {
                    setScreen(SCREEN_CONFIG);
                }
                else if (m_value_num < m_value_count - 1)
                {
                    m_value_num++;
                    setScreen(SCREEN_CONFIG);
                }
                else
                {
                    setScreen(SCREEN_CONFIG_BASE);
                }
                return true;
            }
        }
        else if (m_screen_num == SCREEN_CONFIG)
        {
            return handleValue(button_num,event_type);
        }
    }

    return false;
}
