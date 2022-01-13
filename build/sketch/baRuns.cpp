//-----------------------------------
// baRuns.cpp - see baExterns.h
//-----------------------------------
#include "bilgeAlarm.h"
#include "baExterns.h"
#include <myIOTLog.h>


typedef struct {
    time_t  tm;
    uint16_t dur;
    uint16_t flags;   // == emergency pump
} runHistory_t;


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



uint32_t getStartDuration()
{
    return start_duration;
}


void setRunFlags(uint16_t flags)
{
    run_history[run_head].flags |= flags;
}


extern void updateStartDuration()
{
    start_duration = time(NULL);
    // static time_t last_duration = 0;
    // if (last_duration != start_duration)
    // {
    //     last_duration = start_duration;
    //     LOGD("updateStartDuration(%f)",start_duration);
    // }
}

void clearRuns()
{
    run_head = 0;
    memset(run_history,0,MAX_RUN_HISTORY * sizeof(runHistory_t));
    bilge_alarm->setTime(ID_TIME_LAST_RUN,0);
    bilge_alarm->setInt(ID_SINCE_LAST_RUN,0);
    bilge_alarm->setInt(ID_DUR_LAST_RUN,0);
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
    bilge_alarm->setTime(ID_TIME_LAST_RUN,now);
    bilge_alarm->setInt(ID_SINCE_LAST_RUN,(int32_t)now);
    bilge_alarm->setInt(ID_DUR_LAST_RUN,0);
}

void endRun()
{
    time_t now = time(NULL);
    int duration = now - start_duration;
    if (duration == 0) duration = 1;
    LOGU("PUMP OFF(%d) %d secs",run_head,duration);
    runHistory_t *ptr = &run_history[run_head];
    ptr->dur = duration;
    bilge_alarm->setInt(ID_DUR_LAST_RUN,duration);
    start_duration = 0;
    if (++run_head >= MAX_RUN_HISTORY)
        run_head = 0;
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

    uint32_t cutoff;
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

    int count = 0;
    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];

    while (ptr->tm && ptr->tm >= cutoff)
    {
        count++;
        // LOGD("    Counting run(%d) at %s",count,timeToString(ptr->tm).c_str());
        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    // LOGD("countRuns(%d) returning %d",hours,count);
    return count;
}


void setRunWindow(int type, int num)
{
    #if DEBUG_RUNS
        LOGD("setRunWindow(%d,%d)",type,num);
    #endif

    int count = 0;
    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];

    // so if the error is set to 4 per hour, we want to return the
    // time of the 4th one back

    time_t rslt = 0;
    while (ptr->tm)
    {
        count++;
        if (count == num)
        {
            rslt = ptr->tm;
            break;
        }

        #if DEBUG_RUNS
            LOGD("setRunWindow(%d,%d) skipping(%d) at %s",type,num,count,timeToString(ptr->tm).c_str());
        #endif

        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    // should never get here, since the history just triggered an error

    LOGU("setRunWindow(%d,%d)=%s",type,num,timeToString(rslt));
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

    // this loop should be abstracted

    int head = run_head - 1;
    if (head < 0)
        head = MAX_RUN_HISTORY-1;
    runHistory_t *ptr = &run_history[head];
    int count = 0;

    while (ptr->tm)
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
        rslt += String(head);
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
        rslt += ptr->flags ? "EMERGENCY" : "&nbsp;";
        rslt += "</td></tr>\n";

        head--;
        if (head < 0)
            head = MAX_RUN_HISTORY-1;
        if (head == run_head)
            break;
        ptr = &run_history[head];
    }

    if (!count)
        rslt += "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    else
        rslt += "</table>";

    rslt += "</body>\n";
    return rslt;
}
