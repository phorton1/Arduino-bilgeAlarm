//-----------------------------------------
// baScreen.h
//-----------------------------------------

#pragma once

#include "bilgeAlarm.h"


class baScreen
{
    public:

        baScreen(int *button_pins);

        void loop();
            // The screen maintains itself if times or
            // config options change.

        bool onButton(int button_num, int event_type);
            // called from baButtons::loop()
            // return true if the BUTTON_TYPE_PRESS was handled
            // return value ignored otherwise ..

        void onValueChanged(const myIOTValue *value);
            // called from bilgeAlarm/myIOTDevice when any value changes

    private:

        bool m_started;
        int m_menu_mode;
        int m_screen_num;
        int m_prev_screen;

        uint32_t m_last_time;
        uint32_t m_last_screen_time;
        uint32_t m_last_state;
        uint32_t m_last_since;

        int m_backlight;
        uint32_t m_backlight_time;

        // int m_last_pref_value;

        void backlight(int val);
        void setScreen(int screen_num);

};


extern baScreen *ba_screen;
