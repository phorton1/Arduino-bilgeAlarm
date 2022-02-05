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

#define DEBUG_HIST  0

#define TEST_TIMES  0
    // define allows me to test in compressed time where
    // one hour == 1 minute, one day=3 minutes, and one week==5 minutes

#define HIST_DB_FILENAME    "/run_history.dat"


time_t baHistory::m_start_duration;
int baHistory::m_run_head;
int baHistory::m_prev_head;
time_t baHistory::m_clear_day_time;
time_t baHistory::m_clear_hour_time;
runHistory_t baHistory::m_run_history[MAX_RUN_HISTORY];

baHistory ba_history;


void baHistory::init()
    // initialize memory variables
{
    m_run_head = 0;
    m_prev_head = -1;
    m_clear_day_time = 0;
    m_clear_hour_time = 0;
    memset(m_run_history,0,MAX_RUN_HISTORY * sizeof(runHistory_t));
}


void baHistory::clearHistory()
{
    LOGI("baHistory::clearHistory()");
    proc_entry();

    #ifdef WITH_SD
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
    // initialize from the file if exists.  This should set last_run and duration as well,
    // and it would be nice if it also could retrieve the "time windows" for error handling
    // to prevent re-alarm after reboot.
{
    LOGI("baHistory::initHistory()");
    proc_entry();

    init();

    #ifdef WITH_SD
        if (!bilgeAlarm::hasSD())
        {
            LOGE("NO SD CARD in baHistory::initRTCMemory(DATABASE_ON_SD)");
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
                uint32_t pos = 0;
                uint32_t size = file.size();
                uint32_t amt = size;
                uint32_t max = (MAX_RUN_HISTORY-1) * sizeof(runHistory_t);
                if (size > max)
                {
                    pos = size - max;
                    amt = max;
                }

                LOGD("reading %d bytes from %s(%d) at pos=%d",amt,HIST_DB_FILENAME,size,pos);
                if (!file.seek(pos))
                {
                    LOGE("Could not seek(%d) in SD %s",pos,HIST_DB_FILENAME);
                }
                else
                {
                    uint32_t got = file.read((uint8_t *)&m_run_history, amt);
                    if (got != amt)
                    {
                        LOGE("Error reading SD %s expected %d got %d",HIST_DB_FILENAME,amt,got);
                        init();
                    }
                    else
                    {
                        int num = amt/sizeof(runHistory_t);
                        LOGD("got %d runs from SD %s",num,HIST_DB_FILENAME);
                        m_run_head = num;
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





uint32_t baHistory::getStartDuration() const
{
    return m_start_duration;
}


void baHistory::setRunFlags(uint16_t flags)
{
    m_run_history[m_prev_head].flags |= flags;
}


void baHistory::updateStartDuration()
{
    m_start_duration = time(NULL);
}


void baHistory::startRun()
{
    time_t now = time(NULL);
    m_start_duration = now;
    LOGU("PUMP ON(%d)",m_run_head);
    runHistory_t *ptr = &m_run_history[m_run_head];
    ptr->tm = now;
    ptr->dur = 0;
    ptr->flags = 0;
    m_prev_head = m_run_head;
    m_run_head++;
    if (m_run_head >= MAX_RUN_HISTORY)
        m_run_head = 0;
}


uint32_t baHistory::endRun()
{
    time_t now = time(NULL);
    int duration = now - m_start_duration + 1;
    LOGU("PUMP OFF(%d) %d secs",m_prev_head,duration);
    runHistory_t *ptr = &m_run_history[m_prev_head];
    ptr->dur = duration;
    m_start_duration = 0;

    #ifdef WITH_SD
        if (!bilgeAlarm::hasSD())
        {
            LOGE("NO SD CARD in baHistory::endRun(DATABASE_ON_SD)");
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
                file.write((const uint8_t *)ptr,sizeof(runHistory_t));
                file.close();
            }
        }
    #endif

    return duration;
}



void baHistory::initIterator(int *iterator) const
{
    *iterator = m_run_head;
}


const runHistory_t *baHistory::getNext(int *iterator) const
{
    int head = *iterator;
    head -= 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    if (head == m_run_head)
        return NULL;
    if (!m_run_history[head].tm)
        return NULL;

    *iterator = head;
    return &m_run_history[head];
}


const runHistory_t *baHistory::getPrev(int *iterator) const
{
    int head = *iterator;
    head++;
    if (head >=  MAX_RUN_HISTORY)
        head = 0;
    if (head == m_run_head)
        return NULL;
    if (!m_run_history[head].tm)
        return NULL;
    *iterator = head;
    return &m_run_history[head];
}



int baHistory::countRuns(int count_how) const
{
    // LOGD("countRuns(%d) m_run_head=%d",hours,m_run_head);
    // can't really debug in here since it is called every second

    uint32_t cutoff = 0;        // COUNT_ALL
    time_t now = time(NULL);

    if (count_how == COUNT_WEEK)
    {
        cutoff = now - (TEST_TIMES ? (5 * 60) : (7 * 24 * 60 * 60));
    }
    else if (count_how == COUNT_DAY)
    {
        cutoff = now - (TEST_TIMES ? (3 * 60) : (24 * 60 * 60));
        if (m_clear_day_time > cutoff)
            cutoff = m_clear_day_time;
    }
    else if (count_how == COUNT_HOUR)
    {
        cutoff = now - (TEST_TIMES ? (1 * 60) : (60 * 60));
        if (m_clear_hour_time > cutoff)
            cutoff = m_clear_hour_time;
    }

    int iter = 0;
    int count = 0;
    initIterator(&iter);
    const runHistory_t *ptr = getNext(&iter);

    while (ptr && ptr->tm >= cutoff)
    {
        count++;
        ptr = getNext(&iter);
    }

    return count;
}


void baHistory::setCountWindow(int type, int num)
{
    #if DEBUG_HIST
        LOGD("setCountWindow(%d,%d)",type,num);
    #endif

    int iter = 0;
    int count = 0;
    initIterator(&iter);
    const runHistory_t *ptr = getNext(&iter);

    while (ptr)
    {
        count++;
        if (count >= num)
            break;
        ptr = getNext(&iter);
    }

    time_t rslt = ptr ? ptr->tm : 0;
    LOGU("setCountWindow(%d,%d)=%s",type,num,timeToString(rslt).c_str());
    if (type == COUNT_DAY)
        m_clear_day_time = rslt;
    else
        m_clear_hour_time = rslt;
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

    int iter = 0;
    int count = 0;
    initIterator(&iter);
    const runHistory_t *ptr = getNext(&iter);

    while (ptr)
    {
        count++;
        if (count == 1)
        {
            rslt += "<b>";
            rslt += String(countRuns(COUNT_ALL));
            rslt += " ";
            rslt +=  bilge_alarm->getName();
            rslt += " History Items</b><br><br>\n";
            rslt += "<table border='1' padding='6' style='border-collapse:collapse'>\n";
            rslt += "<tr><th>num</th><th>time</th><th>dur</th><th>ago</th></th><th>flags</th></tr>\n";
        }

        rslt += "<tr><td>";
        rslt += String(count);
        rslt += "  (";
        rslt += String(iter);
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

        ptr = getNext(&iter);
    }

    if (!count)
        rslt += "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    else
        rslt += "</table>";

    rslt += "</body>\n";
    return rslt;
}
