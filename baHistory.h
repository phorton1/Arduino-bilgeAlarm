//------------------------------------
// baHistory.h
//------------------------------------
// 2024-09-27  modified to use myIOTDataLog and one-time
// conversion of existing run_history.dat to bilge.dataLog
//
// bilgeAlarm wants to be able to quickly
// count the runs for the last hour, day, and week with
// the quirky notion that it knows if one should be added
// to all values (if we are in a run that has not yet
// terminated, so that we can, in fact, set the flags
// beofre adding the run to the log), coupled with the
// fact that the bilgeAlarm has _hour_cutoff and _day_cutoff
// values that it uses to clear errors once they have occurred,
// and also the old uiScreen assumed it had direct in-memory access
// to a certain number of records.
//
// For the latter, I will just remove the history from the
// uiScreen ... it was not a viable way to look at the history
// anyways.



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
            // called from onCustomLink() returns "" or RESPONSE_HANDLED



};  // class baHistory


extern baHistory ba_history;
