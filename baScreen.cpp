//-----------------------------------------
// baScreen.cpp
//-----------------------------------------

#include "baScreen.h"
#include "baButtons.h"
#include <myIOTLog.h>
#include <LiquidCrystal_I2C.h>


#define DEBUG_SCREEN  0


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
#define ALARM_CLEARED_DURATION    4000
    // keep the stats screen on a little longer

#define SECOND_CLOCK_CUTOFF       120   // 3600
    // set to something like 120 to test clock changeover

#define MENU_MODE_MAIN      0     // main mode - main screen and commands
#define MENU_MODE_CONFIG    1     // config mode - modify config options
#define MENU_MODE_ERROR     2     // well, it should be zero now.


#define SCREEN_ERROR            0
#define SCREEN_PRESS_TO_QUIET   1
#define SCREEN_PRESS_TO_CLEAR   2
#define SCREEN_ALARM_CLEARED    3

#define SCREEN_MAIN             4
#define SCREEN_RELAY            5
#define SCREEN_SELFTEST         6
#define SCREEN_CLEAR_HISTORY    7
#define SCREEN_REBOOT           8
#define SCREEN_FACTORY_RESET    9

#define SCREEN_CONFIRM          10

#define SCREEN_CONFIG_BASE      11



const char *screens[] = {

    "%S",                   "%S",                   // 0
    "PRESS ANY KEY TO",     "SILENCE ALARM",        // 1
    "PRESS ANY KEY TO",     "CLEAR ALARM",          // 2
    "ALARM",                "CLEARED",              // 3

    "HOUR %-3d DAY %-3d",   "%-8s %-7s",             // 4
    "PRIMARY PUMP",         "RELAY        %-3S",    // 5
    "SELF TEST",            "         PERFORM",     // 6
    "CLEAR HISTORY",        "         PERFORM",     // 7
    "REBOOT",               "         PERFORM",     // 8
    "FACTORY RESET",        "         PERFORM",     // 9

    "CONFIRM",              "%S",                   // 10

    "CONFIG",               "BASE",                 // 11
};



baScreen *ba_screen = NULL;
baButtons *ba_buttons = NULL;
LiquidCrystal_I2C lcd(0x27,LCD_LINE_LEN,2);   // 20,4);
    // set the LCD address to 0x27 for a 16 chars and 2 line display

// const char *cur_value_id = NULL;


baScreen::baScreen(int *button_pins)
    // The screen maintains itself if times or
    // config options change.
{
    m_started = false;
    m_screen_num = -1;
    m_menu_mode = MENU_MODE_MAIN;
    m_prev_screen = 0;
    m_last_time = 0;

    m_last_state = 0;
    m_last_since = 0;

    ba_screen = this;
    ba_buttons = new baButtons(button_pins);

    lcd.init();                      // initialize the lcd
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print(0,bilgeAlarm::getDeviceType());
    lcd.setCursor(0,1);
    lcd.print(bilgeAlarm::getVersion());
}



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

void baScreen::onValueChanged(const myIOTValue *value)
{
    bool needs_draw = false;
    switch (m_screen_num)
    {
        case SCREEN_MAIN:
            needs_draw =
                !strcmp(value->getId(),ID_STATE) ||
                !strcmp(value->getId(),ID_SINCE_LAST_RUN) ||
                !strcmp(value->getId(),ID_NUM_LAST_DAY) ||
                !strcmp(value->getId(),ID_NUM_LAST_HOUR) ||
                !strcmp(value->getId(),ID_NUM_LAST_WEEK);
            break;
    }
    if (needs_draw)
        setScreen(m_screen_num);
}



//-----------------------------------
// implementation
//-----------------------------------

// const char *off_secs(int i)
// {
//     if (i<0) i=0;
//     if (i>255) i=255;
//     if (i)
//         sprintf(screen_num_buf,"%3d",i);
//     else
//         strcpy_P(screen_num_buf,PSTR("off"));
//     return screen_num_buf;
// }
//
// const char *enabled_disabled(int i)
// {
//     return i ?
//         PSTR("disabled") :
//         PSTR("enabled");
// }
//
// const char *start_end(int i)
// {
//     return i ?
//         PSTR("end") :
//         PSTR("start");
// }
//

static const char *off_on(bool b)
{
    return b ? "on" : "off";
}

//
// const char *getPrefValueString(int pref_num,int pref_value)
// {
//     const char *ts;
//     if (pref_num == PREF_DISABLED)
//         ts = enabled_disabled(pref_value);
//     else if (pref_num == PREF_EXTRA_PRIMARY_MODE)
//         ts = start_end(pref_value);
//     else if (pref_num == PREF_END_PUMP_RELAY_DELAY)
//     {
//         sprintf(screen_num_buf,"%3d",pref_value);
//         ts = screen_num_buf;
//     }
//     else
//         ts = off_secs(pref_value);
//     return  ts;
// }
//


void baScreen::backlight(int val)
{
    m_backlight = val;
    m_backlight_time = millis();
    if (val)
        lcd.backlight();
    else
        lcd.noBacklight();
}



static void print_lcd(int lcd_line, int screen_line, ...)
    // resuses myDebug display buffers
{
    va_list var;
    va_start(var, screen_line);
    char buffer[LCD_BUF_LEN];
    vsprintf(buffer,screens[screen_line],var);
    int len = strlen(buffer);
    while (len < LCD_LINE_LEN)
    {
        buffer[len++] = ' ';
    }
    buffer[len] = 0;
    lcd.setCursor(0,lcd_line);
    lcd.print(buffer);
}




void baScreen::setScreen(int screen_num)
{
    // set m_last_screen_time to the last time m_screen_num changed ...
    // thia ia used for auto-rotations

    if (m_screen_num != screen_num)
    {
        m_prev_screen = m_screen_num;
        DBG_SCREEN("setScreen(%d)",screen_num);
        m_last_screen_time = millis();
        m_screen_num = screen_num;
    }

    uint32_t state = bilgeAlarm::getState();
    uint32_t alarm_state = bilgeAlarm::getAlarmState();

    int n0 = m_screen_num * 2;
    int n1 = m_screen_num * 2 + 1;
        // the line numbers for the screen

    switch (m_screen_num)
    {
        // screens with no params

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
                const char *format = "%02d:%02d";
                uint32_t tm = time(NULL);
                uint32_t since = tm - last;

                // we start showing minutes:seconds
                // if it has been more than one hour, we show hours-minutes
                // with a dash

                if (since > SECOND_CLOCK_CUTOFF)
                {
                    format = "%02d-%02d";
                    since /= 60;
                }

                uint32_t place_since = since / 60;
                uint32_t units_since = since % 60;

                if (place_since > 999) place_since = 999;
                sprintf(buf2,format,place_since,units_since);
                m_last_since = since;
            }

            print_lcd(0,n0,hour_count,day_count);
            print_lcd(1,n1,buf,buf2);
            break;
        }

        // COMMANDS

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

        // PREFS

        default:
        {
            int pref_num = m_screen_num - SCREEN_CONFIG_BASE;
            // int pref_value = getPref(pref_num);
            print_lcd(0,n0);
            print_lcd(1,n1);    // ,getPrefValueString(pref_num,pref_value));
            break;
        }
    }

}   // setScreen()



void baScreen::loop()
    // The screen maintains itself for auto rotation
    // or if times or values changes
{
    if (!m_started)
    {
        m_started = true;
        setScreen(SCREEN_MAIN);
    }

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
            if (secs && now > m_backlight_time + secs * 1000)
                backlight(0);
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
                else if (m_last_since)
                {
                    uint32_t since = time(NULL) - bilge_alarm->getInt(ID_SINCE_LAST_RUN);
                    if (since > SECOND_CLOCK_CUTOFF)
                        since /= 60;
                    if (since != m_last_since)
                    {
                        m_last_since = since;
                        setScreen(m_screen_num);
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

        ba_buttons->loop();

        // save the state for comparison next time through

        m_last_state = state;
    }

}   // baScreen::loop()





bool baScreen::onButton(int button_num, int event_type)
    // called from baButtons::loop()
{
    DBG_SCREEN("baScreen::onButton(%d,%d)",button_num,event_type);

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
        m_backlight_time = millis();
        return true;
    }

    // any keystroke keeps tha backlight alive

    m_backlight_time = millis();

    //---------------------------
    // COMMANDS
    //---------------------------

    if (m_menu_mode == MENU_MODE_MAIN)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                m_menu_mode = MENU_MODE_CONFIG;
                setScreen(SCREEN_CONFIG_BASE);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                if (m_screen_num == SCREEN_FACTORY_RESET)
                    setScreen(SCREEN_MAIN);
                else
                    setScreen(m_screen_num+1);
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
                    // bilgeAlarm::selfTest();
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
        else if (button_num == 1)
        {
            if (m_screen_num == SCREEN_CONFIRM)
                setScreen(m_prev_screen);
        }
    }

    //---------------------------
    // CONFIG
    //---------------------------

    else // if (m_menu_mode == MENU_MODE_CONFIG)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                m_menu_mode = MENU_MODE_MAIN;
                setScreen(SCREEN_MAIN);
                return true;
            }
            else if (event_type == BUTTON_TYPE_CLICK)
            {
                // if (m_screen_num == SCREEN_CONFIG_BASE + NUM_PREFS - 2)
                //     setScreen(SCREEN_PREF_BASE);
                // else
                //     setScreen(m_screen_num+1);
                return true;
            }
        }

        // handle pref modifications with repeat and release events
        // always returns false

        // else
        // {
        //     int pref_num = m_screen_num - SCREEN_PREF_BASE + 1;
        //     if (event_type == BUTTON_TYPE_PRESS)
        //         m_last_pref_value = getPref(pref_num);
        //
        //     // set the pref upon release
        //     // otherwise inc or dec
        //
        //     if (event_type == BUTTON_TYPE_CLICK)
        //         setPref(pref_num,m_last_pref_value);
        //     else
        //     {
        //         int inc = (button_num == 1) ? -1 : 1;
        //         int pref_max = getPrefMax(pref_num);
        //         m_last_pref_value += inc;
        //
        //         // constrain backlight to 0,30..255
        //         // to prevent pesky behavior
        //
        //         if (pref_num == PREF_BACKLIGHT_SECS)
        //         {
        //             if (inc<0 && m_last_pref_value == 29)
        //                 m_last_pref_value = 0;
        //             if (inc>0 && m_last_pref_value == 1)
        //                 m_last_pref_value = 30;
        //         }
        //
        //
        //         if (m_last_pref_value < 0)
        //             m_last_pref_value = 0;
        //         if (m_last_pref_value > pref_max)
        //             m_last_pref_value = pref_max;
        //
        //     }
        //
        //     int n1 = m_screen_num * 2 + 1;
        //     print_lcd(1,n1,getPrefValueString(pref_num,m_last_pref_value));
        // }
    }

    return false;
}
