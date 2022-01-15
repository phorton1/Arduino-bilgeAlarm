//-----------------------------------------
// uiScreen.h
//-----------------------------------------

#pragma once

#include "bilgeAlarm.h"
#include "baExterns.h"


class uiScreen
{
    public:

        uiScreen(int *button_pins);

        void loop();
            // The screen maintains itself if times or
            // config options change.

        bool onButton(int button_num, int event_type);
            // called from uiButtons::loop()
            // return true if the BUTTON_TYPE_PRESS was handled
            // return value ignored otherwise ..

        void onValueChanged(const myIOTValue *value, valueStore from);
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

        int m_hist_num;
        int m_hist_iter;
        runHistory_t *m_hist_ptr;


        // int m_last_pref_value;

        void backlight(int val);
        void setScreen(int screen_num);

};


extern uiScreen *ui_screen;
