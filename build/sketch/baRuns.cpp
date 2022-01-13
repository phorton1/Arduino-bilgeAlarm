//-----------------------------------
// baRuns.cpp - see baExterns.h
//-----------------------------------
#include "bilgeAlarm.h"
#include "baExterns.h"
#include <myIOTLog.h>


static time_t start_duration;
    // The "duration" that pump1 is "on"
    // INCLUDES the "extra_time" if EXTRA_MODE(0) "at_start", so
    // the ERR_RUN_TIME and CRIT_RUN_TIME values must be larger than EXTRA_RUN_TIME
    // if using EXTRA_RUN_MODE(0) "at_start"
static time_t clear_day_time;
static time_t clear_hour_time;
    // if a clearError is done, these prevents countRuns() from returning
    // any runs before this time.


static runHistory_t run_history[MAX_RUN_HISTORY];
static int run_head = 0;
static int prev_head = -1;


uint32_t getStartDuration()
{
    return start_duration;
}


void setRunFlags(uint16_t flags)
{
    run_history[prev_head].flags |= flags;
}


void updateStartDuration()
{
    start_duration = time(NULL);
}

void clearRuns()
{
    run_head = 0;
    prev_head = -1;
    memset(run_history,0,MAX_RUN_HISTORY * sizeof(runHistory_t));

}


void startRun()
{
    time_t now = time(NULL);
    start_duration = now;
    LOGU("PUMP ON(%d)",run_head);
    runHistory_t *ptr = &run_history[run_head];
    ptr->tm = now;
    ptr->dur = 0;
    ptr->flags = 0;
    prev_head = run_head;
    run_head++;
    if (run_head >= MAX_RUN_HISTORY)
        run_head = 0;
}


uint32_t endRun()
{
    time_t now = time(NULL);
    int duration = now - start_duration;
    if (duration == 0) duration = 1;
    LOGU("PUMP OFF(%d) %d secs",prev_head,duration);
    runHistory_t *ptr = &run_history[prev_head];
    ptr->dur = duration;
    start_duration = 0;
    return duration;
}



void initRunIterator(int *iterator)
{
    *iterator = run_head;
}


runHistory_t *getNextRun(int *iterator)
{
    int head = *iterator;
    head -= 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    if (head == run_head)
        return NULL;
    if (!run_history[head].tm)
        return NULL;

    *iterator = head;
    return &run_history[head];
}


runHistory_t *getPrevRun(int *iterator)
{
    int head = *iterator;
    head++;
    if (head >=  MAX_RUN_HISTORY)
        head = 0;
    if (head == run_head)
        return NULL;
    if (!run_history[head].tm)
        return NULL;
    *iterator = head;
    return &run_history[head];
}



int countRuns(int type)
    // note that this does NOT count the current run which is happening
    // it only counts fully commited runs, so per_day and per_hour alarms
    // happen when the pump turns OFF.   Due to the alarm clearing strategy,
    // the and week counts shown after a clear will more or less be zeroed,
    // though the history still contains all the runs.
{
    // LOGD("countRuns(%d) run_head=%d",hours,run_head);
    // can't really debug in here since it is called every second

    uint32_t cutoff = 0;        // COUNT_ALL
    time_t now = time(NULL);

    if (type == COUNT_WEEK)
    {
        cutoff = now - (TEST_TIMES ? (5 * 60) : (7 * 24 * 60 * 60));
    }
    else if (type == COUNT_DAY)
    {
        cutoff = now - (TEST_TIMES ? (3 * 60) : (24 * 60 * 60));
        if (clear_day_time > cutoff)
            cutoff = clear_day_time;
    }
    else // type == COUNT_HOUR
    {
        cutoff = now - (TEST_TIMES ? (1 * 60) : (60 * 60));
        if (clear_hour_time > cutoff)
            cutoff = clear_hour_time;
    }

    int iter = 0;
    int count = 0;
    initRunIterator(&iter);
    runHistory_t *ptr = getNextRun(&iter);

    while (ptr && ptr->tm >= cutoff)
    {
        count++;
        ptr = getNextRun(&iter);
    }

    return count;
}


void setRunWindow(int type, int num)
{
    #if DEBUG_RUNS
        LOGD("setRunWindow(%d,%d)",type,num);
    #endif

    int iter = 0;
    int count = 0;
    initRunIterator(&iter);
    runHistory_t *ptr = getNextRun(&iter);

    while (ptr)
    {
        count++;
        if (count >= num)
            break;
        ptr = getNextRun(&iter);
    }

    time_t rslt = ptr ? ptr->tm : 0;
    LOGU("setRunWindow(%d,%d)=%s",type,num,timeToString(rslt).c_str());
    if (type == COUNT_DAY)
        clear_day_time = rslt;
    else
        clear_hour_time = rslt;
}




// might be a stack overflow
// building this on the stack

String historyHTML()
{
    String rslt = "<head>\n";
    rslt += "<title>";
    rslt += bilge_alarm->getName();
    rslt += " History</title>\n";
    rslt += "<body>\n";
    rslt += "<style>\n";
    rslt += "th, td { padding-left: 12px; padding-right: 12px; }\n";
    rslt += "</style>\n";

    int iter = 0;
    int count = 0;
    initRunIterator(&iter);
    runHistory_t *ptr = getNextRun(&iter);

    while (ptr)
    {
        count++;
        if (count == 1)
        {
            rslt += "<b>";
            rslt +=  bilge_alarm->getName();
            rslt += " History</b><br><br>\n";
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

        if (ptr->flags & ALARM_STATE_EMERGENCY) rslt += "EMERGENCY ";
        if (ptr->flags & ALARM_STATE_CRITICAL) rslt += "CRITICAL ";
        if (ptr->flags & ALARM_STATE_ERROR) rslt += "ERROR ";
        if (ptr->flags & STATE_CRITICAL_TOO_LONG) rslt += "WAY TOO LONG ";
        else if (ptr->flags & STATE_TOO_LONG) rslt += "TOO LONG ";
        if (ptr->flags & STATE_TOO_OFTEN_HOUR) rslt += "TOO OFTEN ";
        if (ptr->flags & STATE_TOO_OFTEN_DAY) rslt += "TOO OFTEN DAY";
        rslt += "</td></tr>\n";

        ptr = getNextRun(&iter);
    }

    if (!count)
        rslt += "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    else
        rslt += "</table>";

    rslt += "</body>\n";
    return rslt;
}
