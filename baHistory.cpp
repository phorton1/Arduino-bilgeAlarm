//-----------------------------------
// baHistory.cpp
//-----------------------------------
// This object will function without an SD card, but
// the history will not be persistent through reboots.
//
// Note that this used to use RTC_NOINIT_ATTR memory
// and myIOTDevice::onInitRTCMemory() to keep the history
// between reboots.

#include "baHistory.h"
#include "bilgeAlarm.h"
#include <myIOTLog.h>
#include <SD.h>

#define DEBUG_COUNT  0

#define TEST_TIMES  0
    // define allows me to test in compressed time where
    // one hour == 1 minute, one day=3 minutes, and one week==5 minutes

#define HIST_DB_FILENAME    "/run_history.dat"

int baHistory::m_num_items;
historyItem_t baHistory::m_history[MAX_RUN_HISTORY];

baHistory ba_history;


void baHistory::init()
    // initialize memory variables
{
    m_num_items = 0;
    memset(m_history,0,MAX_RUN_HISTORY * sizeof(historyItem_t));
}


void baHistory::clearHistory()
{
    LOGI("baHistory::clearHistory()");
    proc_entry();

    #if WITH_SD
        if (!bilgeAlarm::hasSD())
        {
            LOGE("NO SD CARD in baHistory::clearHistory(DATABASE_ON_SD)");
        }
        else if (SD.exists(HIST_DB_FILENAME))
        {
            LOGW("clearing baHistory from SD %s",HIST_DB_FILENAME);
            SD.remove(HIST_DB_FILENAME);
        }
        else
        {
            LOGD("Note: SD %s not found",HIST_DB_FILENAME);
        }
    #endif

    init();
    proc_leave();
}



void baHistory::initHistory()
{
    LOGI("baHistory::initHistory()");
    proc_entry();

    init();

    #if WITH_SD
        if (!bilgeAlarm::hasSD())
        {
            LOGW("NO SD CARD in initHistory()");
        }
        else if (SD.exists(HIST_DB_FILENAME))
        {
            LOGW("initing baHistory from SD %s",HIST_DB_FILENAME);
            File file = SD.open(HIST_DB_FILENAME, FILE_READ);
            if (!file)
            {
                LOGE("Could not open SD %s for reading",HIST_DB_FILENAME);
            }
            else
            {
                uint32_t size = file.size();
                uint32_t num = size / sizeof(historyItem_t);
                if (num > MAX_RUN_HISTORY-1)
                    num = MAX_RUN_HISTORY-1;
                uint32_t amt = num * sizeof(historyItem_t);
                uint32_t pos = size - amt;

                LOGD("reading %d bytes from %s(%d) at pos=%d",amt,HIST_DB_FILENAME,size,pos);
                if (!file.seek(pos))
                {
                    LOGE("Could not seek(%d) in SD %s",pos,HIST_DB_FILENAME);
                }
                else
                {
                    uint32_t got = file.read((uint8_t *)&m_history, amt);
                    if (got != amt)
                    {
                        LOGE("Error reading SD %s expected %d got %d",HIST_DB_FILENAME,amt,got);
                        init();
                    }
                    else
                    {
                        LOGD("got %d runs from SD %s",num,HIST_DB_FILENAME);
                        m_num_items = num;
                    }
                }
                file.close();
            }
        }
        else
        {
            LOGD("Note: SD %s not found",HIST_DB_FILENAME);
        }
    #endif

    proc_leave();
}



void baHistory::addHistory(uint32_t dur, uint16_t flags)
{
    proc_entry();

    time_t now = time(NULL);
    LOGD("addHistory[%d] dur(%d) flags(0x%04x) %s",m_num_items,dur,flags,timeToString(now).c_str());

    if (m_num_items >= MAX_RUN_HISTORY)
    {
        memcpy(&m_history[0],&m_history[1],(MAX_RUN_HISTORY-1) * sizeof(historyItem_t));
        m_num_items--;
    }

    historyItem_t *ptr = &m_history[m_num_items++];
    ptr->tm = time(NULL);
    ptr->dur = dur;
    ptr->flags = flags;

    #if WITH_SD
        if (!bilgeAlarm::hasSD())
        {
            LOGW("NO SD CARD in addHistory()");
        }
        else
        {
            File file = SD.open(HIST_DB_FILENAME, FILE_APPEND);
            if (!file)
            {
                LOGE("Could not open SD %s for append",HIST_DB_FILENAME);
            }
            else
            {
                file.write((const uint8_t *)ptr,sizeof(historyItem_t));
                file.close();
            }
        }
    #endif

    proc_leave();
}


void baHistory::countRuns(int add, int *hour_count, int *day_count, int *week_count)
{
    proc_entry();

    *hour_count = add;
    *day_count = add;
    *week_count = add;

    if (!m_num_items)
        return;

    time_t now = time(NULL);

    uint32_t week_cutoff = now - (7 * 24 * 60 * 60);
    uint32_t day_cutoff  = now - (24 * 60 * 60);
    uint32_t hour_cutoff = now - (60 * 60);
    if (day_cutoff < bilge_alarm->_day_cutoff)
        day_cutoff = bilge_alarm->_day_cutoff;
    if (hour_cutoff < bilge_alarm->_hour_cutoff)
        hour_cutoff = bilge_alarm->_hour_cutoff;

    #if DEBUG_COUNT
        LOGD("week_cutoff(%d) day_cutoff(%d) hour_cutoff(%d)",week_cutoff,day_cutoff,hour_cutoff);
    #endif

    bool done = 0;
    for (int i=0; i<m_num_items && !done; i++)
    {
        const historyItem_t *ptr = getItem(i);
        if (ptr->tm >= week_cutoff)
        {
            (*week_count)++;
            if (ptr->tm >= day_cutoff)
                (*day_count)++;
            if (ptr->tm >= hour_cutoff)
                (*hour_count)++;
        }
        else
        {
            done = 1;
        }

        #if DEBUG_COUNT
            LOGD("done(%d) item(%s) dur(%d) tm(%d) week(%d) day(%d) month(%d)",
                 done,timeToString(ptr->tm).c_str(),ptr->dur,ptr->tm,*week_count,*day_count,*hour_count);
        #endif
    }

    proc_leave();

}



String baHistory::getHistoryHTML() const
    // building this on the stack ?!?!?
    // PRH - should test this with 256 entries
    // would be better to be able to write this
    // to the socket. web_server.streamFile(file, getContentType(path));
{
    String rslt = "<head>\n";
    rslt += "<title>";
    rslt += bilge_alarm->getName();
    rslt += " History =</title>\n";
    rslt += "<body>\n";
    rslt += "<style>\n";
    rslt += "th, td { padding-left: 12px; padding-right: 12px; }\n";
    rslt += "</style>\n";

    if (m_num_items)
    {
        rslt += "<b>";
        rslt += String(m_num_items);
        rslt += " ";
        rslt +=  bilge_alarm->getName();
        rslt += " History Items</b><br><br>\n";
        rslt += "<table border='1' padding='6' style='border-collapse:collapse'>\n";
        rslt += "<tr><th>num</th><th>time</th><th>dur</th><th>ago</th></th><th>flags</th></tr>\n";

        int count = 0;
        for (int i=0; i<m_num_items; i++)
        {
             count++;
             const historyItem_t *ptr = getItem(i);

            rslt += "<tr><td>";
            rslt += String(count);
            rslt += "  (";
            rslt += String(i);
            rslt += ")";
            rslt += "</td><td>";
            rslt += timeToString(ptr->tm);
            rslt += "</td><td align='center'>";
            rslt += String(ptr->dur);
            rslt += "</td><td>";

            // anything over a 5 years year is considered invalid to deal with potential lack of clock issues

            char buf[128] = "&nbsp;";
            uint32_t since = time(NULL) - ptr->tm;
            if (since < 5 * 365 * 24 * 60 * 60)
            {
                int days = since / (24 * 60 * 60);
                int hours = ((since % (24 * 60 * 60)) / (60 * 60)) % 24;
                int minutes = ((since % (24 * 60 * 60 * 60)) / 60) % 60;
                int secs = since % 60;
                if (days)
                {
                    sprintf(buf,"%d days  %02d:%02d:%02d",days, hours, minutes, secs);
                }
                else
                {
                    sprintf(buf,"%02d:%02d:%02d",hours, minutes, secs);
                }
            }
            rslt += buf;

            rslt += "</td><td align='center'>";

            if (ptr->flags & STATE_EMERGENCY) rslt += "EMERGENCY ";
            if (ptr->flags & STATE_CRITICAL_TOO_LONG) rslt += "WAY TOO LONG ";
            else if (ptr->flags & STATE_TOO_LONG) rslt += "TOO LONG ";
            if (ptr->flags & STATE_TOO_OFTEN_HOUR) rslt += "TOO OFTEN ";
            if (ptr->flags & STATE_TOO_OFTEN_DAY) rslt += "TOO OFTEN DAY";
            rslt += "</td></tr>\n";

        }   // for each item

        rslt += "</table>";
    }
    else
    {
        rslt += "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    }

    rslt += "</body>\n";
    return rslt;
}
