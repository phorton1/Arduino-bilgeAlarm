//-----------------------------------------
// uiScreen.cpp
//-----------------------------------------

#include "uiScreen.h"
#include "uiButtons.h"
#include "baExterns.h"
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

// Errors take over the whole button scheme
//     - any button to suppress alarm
//     - any button to clear alarm
// Otherwise, you can be in a MODE on a SCREEN and it will
//     not change for at least MENU_DELAY seconds.
//     After no activity for MENU_DELAY seconds,
//     the system returns tot he MAIN_MODE and MAIN_SCREEN.
//
// A long click of the left button changes between MAIN and CONFIG modes
// A click of left button cycles through screen within the mode
//     The right button is used to perform commands
//     The middle and right buttons are decrement and increment for values
// In commands that require a confirm,
//     The right button is YES, and the middle button is NO
// A click of the middle button while in MAIN_MODE goes to HISTORY mode
//     the middle button (continued) increments screens
//     the right button decrements screens
//     the left button returns to MAIN mode


#define MENU_MODE_MAIN      0     // main mode - main screen and commands
#define MENU_MODE_HISTORY   1     // stats mode - cycle through history if any
#define MENU_MODE_CONFIG    2     // config mode - modify config options


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

#define SCREEN_HISTORY_BASE     11
#define SCREEN_HISTORY          12

#define SCREEN_CONFIG_BASE      13

const char *screens[] = {

    "%S",                   "%S",                   // 0
    "PRESS ANY KEY TO",     "SILENCE ALARM",        // 1
    "PRESS ANY KEY TO",     "CLEAR ALARM",          // 2
    "ALARM",                "CLEARED",              // 3

    "HOUR %-3d DAY %-3d",   "%-8s%8s",              // 4
    "PRIMARY PUMP",         "RELAY        %-3S",    // 5
    "SELF TEST",            "         PERFORM",     // 6
    "CLEAR HISTORY",        "         PERFORM",     // 7
    "REBOOT",               "         PERFORM",     // 8
    "FACTORY RESET",        "         PERFORM",     // 9

    "CONFIRM",              "%S",                   // 10

    "HISTORY %8d",          "BACK  NEXT  PREV",     // 11
    "%s",                   "%-12s %3d",             // 12

    "CONFIG",               "BASE",                 // 13

};



uiScreen *ui_screen = NULL;
uiButtons *ui_buttons = NULL;
LiquidCrystal_I2C lcd(0x27,LCD_LINE_LEN,2);   // 20,4);
    // set the LCD address to 0x27 for a 16 chars and 2 line display

static int hist_iter;
runHistory_t *hist_ptr;



uiScreen::uiScreen(int *button_pins)
{
    m_started = false;
    m_screen_num = -1;
    m_menu_mode = MENU_MODE_MAIN;
    m_prev_screen = 0;
    m_last_time = 0;

    m_last_state = 0;
    m_last_since = 0;

    ui_screen = this;
    ui_buttons = new uiButtons(button_pins);

    lcd.init();                      // initialize the lcd
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print(0,bilgeAlarm::getDeviceType());
    lcd.setCursor(0,1);
    lcd.print(bilgeAlarm::getVersion());
    backlight(1);
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

void uiScreen::onValueChanged(const myIOTValue *value, valueStore from)
{
    bool needs_draw = false;
    const char *id = value->getId();

    if (from == VALUE_STORE_PROG)
        return;

    // PRH - there should be a minimum of 30 seconds for the backlight
    // so if it turns off you can at least turn it back on

    if (!strcmp(id,ID_BACKLIGHT_SECS))
        m_backlight_time = millis();

    switch (m_screen_num)
    {
        case SCREEN_MAIN:
            needs_draw =
                !strcmp(id,ID_STATE) ||
                !strcmp(id,ID_SINCE_LAST_RUN) ||
                !strcmp(id,ID_NUM_LAST_DAY) ||
                !strcmp(id,ID_NUM_LAST_HOUR) ||
                !strcmp(id,ID_NUM_LAST_WEEK);
            break;
    }
    if (needs_draw)
        setScreen(m_screen_num);
}



//-----------------------------------
// implementation
//-----------------------------------


static const char *off_on(bool b)
{
    return b ? "on" : "off";
}


void uiScreen::backlight(int val)
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




void uiScreen::setScreen(int screen_num)
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

    if (screen_num >= SCREEN_CONFIG_BASE)
    {
        m_menu_mode = MENU_MODE_CONFIG;
        ui_buttons->setRepeatMask(6);
    }
    else if (screen_num >= SCREEN_HISTORY_BASE)
    {
        m_menu_mode = MENU_MODE_HISTORY;
        ui_buttons->setRepeatMask(6);
    }
    else
    {
        m_menu_mode = MENU_MODE_MAIN;
        ui_buttons->setRepeatMask(0);
    }

    uint32_t state = bilgeAlarm::getState();
    uint32_t alarm_state = bilgeAlarm::getAlarmState();

    if (alarm_state)
        ui_buttons->setRepeatMask(0);

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
            print_lcd(0,n0,countRuns(COUNT_ALL));
            print_lcd(1,n1);
            initRunIterator(&hist_iter);
            break;
        case SCREEN_HISTORY:
            if (hist_ptr)
            {
                String dte = timeToString(hist_ptr->tm);
                const char *str = &(dte.c_str())[5];
                print_lcd(0,n0,str);

                // prioritized string

                const char *buf = "";
                uint16_t flags = hist_ptr->flags;
                if (flags & STATE_EMERGENCY)                buf = "EMERGENCY";
                else if (flags & STATE_CRITICAL_TOO_LONG)   buf = "CRIT LONG";
                else if (flags & STATE_TOO_LONG)            buf = "ERR LONG";
                else if (flags & STATE_TOO_OFTEN_DAY)       buf = "ERR NUM/DAY";
                else if (flags & STATE_TOO_OFTEN_HOUR)      buf = "ERR NUM/HOUR";
                print_lcd(1,n1,buf,hist_ptr->dur);
            }
            break;


        // DEFAULT

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



void uiScreen::loop()
{
    if (!m_started)
    {
        m_started = true;
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





bool uiScreen::onButton(int button_num, int event_type)
    // called from uiButtons::loop()
{
    DBG_SCREEN("uiScreen::onButton(%d,%d)",button_num,event_type);

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
        else // if (event_type == BUTTON_TYPE_CLICK)
        {
            if (button_num == 1)
            {
                if (m_screen_num == SCREEN_CONFIRM)
                    setScreen(m_prev_screen);
                else
                    setScreen(SCREEN_HISTORY_BASE);
                return true;
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
        }
    }

    //-----------------------------------
    // STATS
    //-----------------------------------

    else if (m_menu_mode == MENU_MODE_HISTORY)
    {
        if (button_num == 0 )
        {
            setScreen(SCREEN_MAIN);
        }
        else if (button_num == 1)
        {
            hist_ptr = getNextRun(&hist_iter);
            if (hist_ptr)
                setScreen(SCREEN_HISTORY);
        }
        else if (button_num == 2)
        {
            hist_ptr = getPrevRun(&hist_iter);
            if (hist_ptr)
                setScreen(SCREEN_HISTORY);
            else
                setScreen(SCREEN_HISTORY_BASE);
        }
        return true;
    }

    //---------------------------
    // CONFIG
    //---------------------------

    else // (m_menu_mode == MENU_MODE_CONFIG)
    {
        if (button_num == 0)
        {
            if (event_type == BUTTON_TYPE_LONG_CLICK)
            {
                setScreen(SCREEN_MAIN);
                return true;
            }

        }
    }

    return false;
}
