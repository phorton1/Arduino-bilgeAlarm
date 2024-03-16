//------------------------------------
// baHistory.h
//------------------------------------
#pragma once

#include <Arduino.h>
#include <time.h>

#define MAX_RUN_HISTORY    256


typedef struct {
    time_t  tm;
    uint16_t dur;
    uint16_t flags;
} historyItem_t;


class baHistory
{
    public:

        static void initHistory();
            // initialize the history from the SD file if it exists
        static void clearHistory();
            // clear the history

        static int numItems()  { return m_num_items; }
        static const historyItem_t *getItem(int num_recent)
        {
            int at = m_num_items - num_recent - 1;
            return at >= 0 ? &m_history[at] : 0;
        }

        static void addHistory(uint32_t dur, uint16_t flags);
        static void countRuns(int add, int *hour_count, int *day_count, int *week_count);
            // 'add' is a reminder that we are about to add a new item in the main task
            // and want the count as if it was already added.

        String getHistoryHTML() const;

    private:


        static void init();

        static historyItem_t m_history[MAX_RUN_HISTORY];
        static int m_num_items;

};  // class baHistory


extern baHistory ba_history;
