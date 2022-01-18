//-----------------------------------
// baHistory.cpp
//-----------------------------------
// Interesting devlopement regarding the history in RAM.
// I moved it to the RTC_NOINIT_ATTR memory and created
// the onInitRTCMemory() virtual callchain from myIOTDevice::setup
// when it needs to be re-initialied.  See myIOTDevice.cpp for
// more information.
//
// The history SHALL write a database to the SPIFFS (or SD) file system,
// which appends each run as a 64 bit (8 byte) record to the file.
// Upon the call to initRTCMemory(), if present, the last MAX_RUN_HISTORY
// entries from that file shall be read into memory.
//
// I am currently using 416.9K of 1.37M SPIFFS total, which leaves about 800K,
// or enough room to store about 100,000 runs ... but for safety I would probably
// keep 10,000 with a rotation scheme on SPIFFS.  Using the SDCard is another
// option.

#include "baHistory.h"
#include "bilgeAlarm.h"
#include <myIOTLog.h>

#define DEBUG_HIST  0

#define TEST_TIMES  0
    // define allows me to test in compressed time where
    // one hour == 1 minute, one day=3 minutes, and one week==5 minutes


time_t baHistory::m_start_duration;
RTC_NOINIT_ATTR int baHistory::m_run_head;
RTC_NOINIT_ATTR int baHistory::m_prev_head;
RTC_NOINIT_ATTR time_t baHistory::m_clear_day_time;
RTC_NOINIT_ATTR time_t baHistory::m_clear_hour_time;
RTC_NOINIT_ATTR runHistory_t baHistory::m_run_history[MAX_RUN_HISTORY];

baHistory ba_history;

void baHistory::clearHistory()
{
    m_run_head = 0;
    m_prev_head = -1;
    m_clear_day_time = 0;
    m_clear_hour_time = 0;
    memset(m_run_history,0,MAX_RUN_HISTORY * sizeof(runHistory_t));
}

void baHistory::initRTCMemory()
{
    LOGI("baHistory::initRTCMemory()");
    m_run_head = 0;
    m_prev_head = -1;
    m_clear_day_time = 0;
    m_clear_hour_time = 0;
    memset(m_run_history,0,sizeof(m_run_history));
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
    // note that this does NOT count the current run which is happening
    // it only counts fully commited runs, so per_day and per_hour alarms
    // happen when the pump turns OFF.   Due to the alarm clearing strategy,
    // the and week counts shown after a clear will more or less be zeroed,
    // though the history still contains all the runs.
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
    else // count_how == COUNT_HOUR
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




// might be a stack overflow
// building this on the stack

String baHistory::getHistoryHTML() const
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
            int hours = (since % (24 * 60 * 60)) / (60 * 60);
            int minutes = (since % (24 * 60 * 60 * 60)) / 60;
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
