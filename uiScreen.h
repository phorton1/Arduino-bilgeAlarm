//-----------------------------------------
// uiScreen.h
//-----------------------------------------

#pragma once

#include "bilgeAlarm.h"
#include "baHistory.h"


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
        uint32_t m_activity_time;

        // following inited on BASE_SCREEN

        int m_value_num;
        int m_value_count;
        valueIdType *m_value_ids;

        // following inited by initValueMembers
        // on screen change

        myIOTValue *m_value;
        valueIdType m_value_id;
        valueType m_value_type;
        valueStyle m_value_style;
        int m_value_min;
        int m_value_max;
        int m_cur_value;

        // methods

        void backlight(int val);
        void setScreen(int screen_num);

        void initValueMembers();
        bool handleValue(int button_num, int event_type);

};


extern uiScreen *ui_screen;
