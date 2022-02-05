//------------------------------------
// baHistory.h
//------------------------------------
#pragma once

#include <Arduino.h>
#include <time.h>

#define MAX_RUN_HISTORY    256      // 256 * 8 = 2K
    // set to smaller number (i.e 15) for testing circular buffer
    //
    // circular buffer of run history, where each history element
    // is 8 bytes (two 32 bit numbers), the first being the time
    // of the run, and the second being the 16 bit duration of the
    // run in seconds, and a 16 bit "flags" member that maps to the
    // bilge alarm error STATE_CONSTANTS.
    //
    // The buffer MUST be large enough to handle all the runs in a day,
    // or certainly at least more than the error configuration values


#define COUNT_HOUR   0
#define COUNT_DAY    1
#define COUNT_WEEK   2
#define COUNT_ALL    3
    // the type of count to do

typedef struct {
    time_t  tm;
    uint16_t dur;
    uint16_t flags;   // == emergency pump
} runHistory_t;


class baHistory
{
    public:

        void initHistory();
            // initialize the history from the SD file if it exists
        void clearHistory();
            // clear the history

        int countRuns(int count_how) const;
            // count of runs by COUNT constant
        void setCountWindow(int type, int num);
            // sets the time window for COUNT_HOUR and COUNT_DAY
            // to the last num runs (used to clear count errors)

        void startRun();
            // start a run (a new history element)
        void setRunFlags(uint16_t flags);
            // update the flags (STATE_ types) for current/previous run
        uint32_t endRun();
            // end the current run

        uint32_t getStartDuration() const;
        void updateStartDuration();
            // get (and updsate) the duration member variable

        void initIterator(int *iterator) const;
        const runHistory_t *getNext(int *iterator) const;
        const runHistory_t *getPrev(int *iterator) const;
            // iterator over history in memory

        String getHistoryHTML() const;
            // delivers upto MAX_RUN_HISTORY lines of HTML

    private:

        void init();

        static time_t m_start_duration;
        static int m_run_head;
        static int m_prev_head;
        static time_t m_clear_day_time;
        static time_t m_clear_hour_time;
        static runHistory_t m_run_history[MAX_RUN_HISTORY];

};  // class baHistory


extern baHistory ba_history;
