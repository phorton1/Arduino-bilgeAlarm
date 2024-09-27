//-----------------------------------
// baHistory.cpp
//-----------------------------------
// This object will function without an SD card, but
// the history will not be persistent through reboots.
//
// Note that this used to use RTC_NOINIT_ATTR memory
// and myIOTDevice::onInitRTCMemory() to keep the history
// between reboots.

#include "bilgeAlarm.h"
#include "baHistory.h"
#include <myIOTLog.h>
#include <myIOTDataLog.h>
#include <myIOTWebServer.h>
#include <SD.h>

#define DEBUG_COUNT 1

#define WITH_AUTO_COVERSION  1

#define history_filename "/bilge.datalog"
    // agrees with myIOTDataLog

typedef struct {    // includes the timestamp
    uint32_t tm;
    uint32_t dur;
    uint32_t flags;
} historyRec_t;


#define BASE_BUF_SIZE   512
#define HIST_REC_SIZE   sizeof(historyRec_t)
#define STACK_BUF_SIZE  (((BASE_BUF_SIZE + HIST_REC_SIZE-1) / HIST_REC_SIZE) * HIST_REC_SIZE)

logColumn_t  bilge_cols[] = {
    {"dur",	    LOG_COL_TYPE_UINT32,	10,	},
    {"flags",	LOG_COL_TYPE_UINT32,	1,	},
};


baHistory ba_history;
myIOTDataLog data_log("bilge",2,bilge_cols);


#if WITH_AUTO_COVERSION

    typedef struct {
        time_t  tm;
        uint16_t dur;
        uint16_t flags;
    } oldHistoryRec_t;

    #define old_history_filename "/run_history.dat"

    void oneTimeConvertHistory()
    {
        int old_size = sizeof(oldHistoryRec_t);
        int new_size = sizeof(historyRec_t);

        LOGW("oneTimeConvertHistory() old_size(%d) new_size(%d)",old_size,new_size);
        File old_file = SD.open(old_history_filename, FILE_READ);
        if (!old_file)
        {
            LOGE("Could not open %s for reading",old_history_filename);
            return;
        }
        File new_file = SD.open(history_filename, FILE_WRITE);
        if (!new_file)
        {
            old_file.close();
            LOGE("Could not open %s for reading",old_history_filename);
            return;
        }
        int num_recs = old_file.size() / old_size;
        LOGD("    converting %d records",num_recs);

        int old_at = 0;
        int new_at = 0;
        oldHistoryRec_t old_rec;
        historyRec_t new_rec;
        for (int i=0; i<num_recs; i++)
        {
            int bytes = old_file.read((uint8_t *)&old_rec,old_size);
            if (bytes != old_size)
            {
                LOGE("error reading(%d/%d) at(%d) from %s",
                     bytes,
                     old_size,
                     old_at,
                     old_history_filename);
                old_file.close();
                new_file.close();
                return;
            }
            old_at += old_size;

            new_rec.tm = old_rec.tm;
            new_rec.dur = old_rec.dur;
            new_rec.flags = 0;

            if (old_rec.flags & STATE_TOO_OFTEN_HOUR)
                new_rec.flags |= HIST_STATE_TOO_OFTEN_HOUR;
            if (old_rec.flags & STATE_TOO_OFTEN_DAY)
                new_rec.flags |= HIST_STATE_TOO_OFTEN_DAY;
            if (old_rec.flags & STATE_TOO_LONG)
                new_rec.flags |= HIST_STATE_TOO_LONG;
            if (old_rec.flags & STATE_CRITICAL_TOO_LONG)
                new_rec.flags |= HIST_STATE_CRITICAL_TOO_LONG;
            if (old_rec.flags & STATE_EMERGENCY)
                new_rec.flags |= HIST_STATE_EMERGENCY;

            bytes = new_file.write((uint8_t *)&new_rec,new_size);
            if (bytes != new_size)
            {
                LOGE("error write(%d/%d) at(%d) to %s",
                     bytes,
                     new_size,
                     new_at,
                     history_filename);
                old_file.close();
                new_file.close();
                return;
            }
            new_at += new_size;
        }

        old_file.close();
        new_file.close();
        LOGD("    oneTimeConvertHistory() wrote %d bytes to %s",new_at,history_filename);
    }
#endif  // WITH_CONVERSION


void baHistory::initHistory()
    // initialize the history from the SD file if it exists
{
    LOGI("baHistory::initHistory()");
    
    if (!bilgeAlarm::hasSD())
    {
        LOGE("NO SD CARD in baHistory::initHistory()");
        return;
    }

#if WITH_AUTO_COVERSION
    if (SD.exists(old_history_filename) &&
        !SD.exists(history_filename))
        oneTimeConvertHistory();
#endif

    // get the last history from the SD file, if any, and use it
    // to set the bilge alarm last_run values, and also call
    // count runs the first time to set up those values

    File file = SD.open(history_filename,FILE_READ);
    if (file)
    {
        bool ok = 1;
        historyRec_t rec;
        uint32_t size = file.size();
        if (size >= HIST_REC_SIZE)
        {
            uint32_t at = size - HIST_REC_SIZE;
            if (!file.seek(at))
            {
                LOGE("baHistory not seek(%d) in %s",at,history_filename);
                ok = 0;
            }
            else
            {
                int bytes = file.read((uint8_t *)&rec,HIST_REC_SIZE);
                if (bytes != HIST_REC_SIZE)
                {
                    LOGE("baHistory read error(%d/%d) at %d in %s",bytes,HIST_REC_SIZE,at,history_filename);
                    ok = 0;
                }
                else
                {
                    LOGD("initializing last_run to %s dur %d",timeToString(rec.tm).c_str(),rec.dur);
                    bilge_alarm->_time_last_run = rec.tm;
                    bilge_alarm->_since_last_run = (int32_t) rec.tm;
                    bilge_alarm->_dur_last_run = rec.dur;
                }
            }
            file.close();

            if (ok)
                ba_history.countRuns(0);

        }   // size > 0
    }   // file opened
}   // initHistory()



void baHistory::clearHistory()
    // clear the history
{
    LOGI("baHistory::clearHistory()");
    if (!bilgeAlarm::hasSD())
    {
        LOGE("NO SD CARD in baHistory::clearHistory(DATABASE_ON_SD)");
    }
    else if (SD.exists(history_filename))
    {
        LOGW("clearing baHistory from SD %s",history_filename);
        SD.remove(history_filename);
    }
    else
    {
        LOGD("Note: SD %s not found",history_filename);
    }
}



void baHistory::addHistory(uint32_t dur, uint32_t flags)
{
    LOGD("baHistory::addHistory(%d,%d)",dur,flags);
    
    historyRec_t rec;
    rec.dur = dur;
    rec.flags = flags;
    data_log.addRecord((logRecord_t) &rec);
}



bool baHistoryCondition(uint32_t cutoff, uint8_t *rec_ptr)
{
    uint32_t tm = *((uint32_t *)rec_ptr);
    if (tm >= cutoff)
        return true;
    #if 0
        String dt1 = timeToString(tm);
        String dt2 = timeToString(cutoff);
        LOGD("baHistoryCondition(FALSE) at %s < %s",dt1.c_str(),dt2.c_str());
    #endif
    return false;
}


void baHistory::countRuns(int add)
    // 'add' is a reminder that we are about to add a new item in the main task
    // and want the count as if it was already added; uses _hour_cutoff and
    // day cutoff to limit returns for the bilgeAlarm to be able to 'clear'
    // those particular errors.
    //
    // There are only bad choices if there is an error on the SD card.
    // We don't mess with the num_last_ variables if so.
{
    time_t now = time(NULL);
    uint32_t week_cutoff = now - (7 * 24 * 60 * 60);
    uint32_t day_cutoff  = now - (24 * 60 * 60);
    uint32_t hour_cutoff = now - (60 * 60);
    if (day_cutoff < bilge_alarm->_day_cutoff)
        day_cutoff = bilge_alarm->_day_cutoff;
    if (hour_cutoff < bilge_alarm->_hour_cutoff)
        hour_cutoff = bilge_alarm->_hour_cutoff;

    SDBackwards_t iter;
    uint8_t stack_buffer[STACK_BUF_SIZE];
    iter.chunked        = 1;
    iter.client_data    = week_cutoff;                  // cutoff dt
    iter.filename       = history_filename;
    iter.rec_size       = HIST_REC_SIZE;                // 12 bytes
    iter.record_fxn     = baHistoryCondition;
    iter.buffer         = stack_buffer;                 // an even multiple of rec_size
    iter.buf_size       = STACK_BUF_SIZE;
    iter.dbg_level      = 1;                            // 0..2

	if (!startSDBackwards(&iter))
		return;

    // the file is at least open at this point,
    // so, we reset the actual values

    bilge_alarm->_num_last_hour = add;
    bilge_alarm->_num_last_day = add;
    bilge_alarm->_num_last_week = add;

    // and commence with counting
    
	int num_recs;
	const historyRec_t *base_rec = (const historyRec_t *) getSDBackwards(&iter,&num_recs);
	while (num_recs)
	{
        for (int i=num_recs-1; i>=0; i--)
        {
            const historyRec_t *rec = &base_rec[i];
            bilge_alarm->_num_last_week++;
            if (rec->tm >= day_cutoff)
                bilge_alarm->_num_last_day++;
            if (rec->tm >= hour_cutoff)
                bilge_alarm->_num_last_hour++;
        }
 		base_rec = (const historyRec_t *) getSDBackwards(&iter,&num_recs);
	}

    if (iter.file)
        iter.file.close();

    #if DEBUG_COUNT
        LOGD("baHistory::countRuns(%d) returning hour(%d) day(%d) week(%d)",
            add,
            bilge_alarm->_num_last_hour,
            bilge_alarm->_num_last_day,
            bilge_alarm->_num_last_week);
    #endif
}




String baHistory::getHistoryHTML() const
    // not much difference in memory between chunked and non-chunked
    // if fact non-chunked may fragment memory more leading to less
    // available, but not necessarily a lower water mark.
{
    LOGD("getHistoryHTML() buf_size(%d) rec_size(%d) num_buf_recs=%d",STACK_BUF_SIZE,HIST_REC_SIZE,STACK_BUF_SIZE/HIST_REC_SIZE);

    SDBackwards_t iter;
    uint8_t stack_buffer[STACK_BUF_SIZE];
    iter.chunked        = 1;
    iter.client_data    = 0;                            // cutoff 0 = all records
    iter.filename       = history_filename;
    iter.rec_size       = HIST_REC_SIZE;                     // new 12 bytes
    iter.record_fxn     = baHistoryCondition;
    iter.buffer         = stack_buffer;                 // an even multiple of rec_size
    iter.buf_size       = STACK_BUF_SIZE;
    iter.dbg_level      = 1;                            // 0..2

    if (!myiot_web_server->startBinaryResponse("text/html", CONTENT_LENGTH_UNKNOWN))
        return "";

    String text = "<head>\n";
    text += "<title>";
    text += bilge_alarm->getName();
    text += " History =</title>\n";
    text += "<body>\n";
    text += "<style>\n";
    text += "th, td { padding-left: 12px; padding-right: 12px; }\n";
    text += "</style>\n";

    if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
        return "";

    if (!startSDBackwards(&iter))
    {
        const char *msg = "Could not startSDBackwards()";
        myiot_web_server->writeBinaryData(msg,strlen(msg));
        return RESPONSE_HANDLED;
    }

    int num_recs;
    const historyRec_t *base_rec = (const historyRec_t *) getSDBackwards(&iter,&num_recs);
    if (num_recs)
    {
        text = "";
        text += "<b>";
        text +=  bilge_alarm->getName();
        text += " History Items</b><br><br>\n";
        text += "<table border='1' padding='6' style='border-collapse:collapse'>\n";
        text += "<tr><th>num</th><th>time</th><th>dur</th><th>ago</th></th><th>flags</th></tr>\n";
        if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
        {
            if (iter.file)
                iter.file.close();
            return "";
        }
    }

    int count = 0;
    while (num_recs)
    {
        text = "";
        for (int i=num_recs-1; i>=0; i--)
        {
            const historyRec_t *rec = &base_rec[i];

            count++;

            text += "<tr><td>";
            text += String(count);
            text += "</td><td>";
            text += timeToString(rec->tm);
            text += "</td><td align='center'>";
            text += String(rec->dur);
            text += "</td><td>";

            // anything over a 5 years year is considered invalid to deal with potential lack of clock issues

            char buf[128] = "&nbsp;";
            uint32_t since = time(NULL) - rec->tm;
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
            text += buf;

            text += "</td><td align='center'>";

            if (rec->flags & HIST_STATE_EMERGENCY) text += "EMERGENCY ";
            if (rec->flags & HIST_STATE_CRITICAL_TOO_LONG) text += "WAY TOO LONG ";
            else if (rec->flags & HIST_STATE_TOO_LONG) text += "TOO LONG ";
            if (rec->flags & HIST_STATE_TOO_OFTEN_HOUR) text += "TOO OFTEN ";
            if (rec->flags & HIST_STATE_TOO_OFTEN_DAY) text += "TOO OFTEN DAY";
            text += "</td></tr>\n";

        }   // for each record backwards

        if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
        {
            if (iter.file)
                iter.file.close();
            return "";
        }

        base_rec = (const historyRec_t *) getSDBackwards(&iter,&num_recs);
    }

    if (count)
    {
        text = "</table>";
    }
    else
    {
        text = "THERE IS NO HISTORY OF PUMP RUNS AT THIS TIME\n";
    }

    text += "</body>\n";
    if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
    {
        if (iter.file)
            iter.file.close();
        return "";
    }

    if (iter.file)
        iter.file.close();

    myiot_web_server->finishBinaryResponse();
    LOGD("getHistoryHTML() sent %d records",count);
    return RESPONSE_HANDLED;
}


