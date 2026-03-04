//------------------------------------
// baHistory.h
//------------------------------------

#pragma once

#define HIST_STATE_TOO_OFTEN_HOUR       0x0001
#define HIST_STATE_TOO_OFTEN_DAY        0x0002
#define HIST_STATE_TOO_LONG             0x0004
#define HIST_STATE_CRITICAL_TOO_LONG    0x0008
#define HIST_STATE_EMERGENCY            0x0010



class baHistory
{
    public:

        void initHistory();
            // initialize the history from the SD file if it exists
        void clearHistory();
            // clear the history

        void addHistory(uint32_t dur, uint32_t flags);
        void countRuns(int add);
            // sets bilge_alarm->_num_last_week, _num_last_day, and _num_last_hour
            // 'add' is a reminder that we are about to add a new item in the main task
            // and want the count as if it was already added; uses _hour_cutoff and
            // day cutoff to limit returns for the bilgeAlarm to be able to 'clear'
            // those particular errors.

        String getHistoryHTML() const;
            // returns a displayable webpage with a table of runs
            // called from onCustomLink() returns "" or RESPONSE_HANDLED

        String getBilgeChartHeader();
            // returns a String containing the json used to create a chart
        String sendBilgeChartData(uint32_t secs);
            // sends the chart data to the myiot_web_server and returns
            // RESPONSE_HANDLED. 0 means all.

};  // class baHistory


extern baHistory ba_history;
