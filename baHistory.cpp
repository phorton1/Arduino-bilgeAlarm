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

#define DEBUG_COUNT 0
#define DEBUG_SEND_DATA 1
#define ALT_DATA_FORMAT  1

#define WITH_AUTO_COVERSION  1

//---------------------------------------------------------------
// the data_log is naturally associated with the baHistory
// but is externed to bilgeAlarm.cpp for use in onCustomLink()

#define history_filename    "/bilgeAlarm.datalog"
    // MUST agree with data_log.dataFilename()

#define BASE_BUF_SIZE   512
#define HIST_REC_SIZE   sizeof(historyRec_t)
#define STACK_BUF_SIZE  (((BASE_BUF_SIZE + HIST_REC_SIZE-1) / HIST_REC_SIZE) * HIST_REC_SIZE)
    // 516


typedef struct {    // includes the timestamp
    uint32_t tm;    // which is set by myIOTDataLog::addRecord()
    uint32_t dur;
    uint32_t flags;
} historyRec_t;


logColumn_t  bilge_cols[] = {
    {"dur",	    LOG_COL_TYPE_UINT32,	10,	},
    {"flags",	LOG_COL_TYPE_UINT32,	1,	},
};

baHistory ba_history;
myIOTDataLog data_log("bilgeAlarm",2,bilge_cols);
    // externed to bilgeAlarm.cpp for use in onCustomLink


//----------------------------------

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


//--------------------------------
// implementation
//--------------------------------

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



//------------------------------------------------------------
// onCustomLink chart Support
//------------------------------------------------------------
// shoehorn what I want to display for the bilgeAlarm chart into
// what I built to display line charts generally for frigeController.ge.
//
// I did a little experimentation with bar charts, but generally
// think that they are unsuitable; at least as far as I got them
// to work, they looked ok-ish, but zooming messed up the axes tick stuff.


extern void addJsonVal(String &rslt, const char *field, String val, bool quoted, bool comma, bool cr);
    // in myIOTDataLog.cpp



String baHistory::getBilgeChartHeader()
{
	String rslt = "{\n";

	addJsonVal(rslt,"name","bilgeAlarm",true,true,true);
	addJsonVal(rslt,"num_cols",String(2),false,true,true);
    addJsonVal(rslt,"reverse_canvases",String(0),false,true,true);
#if ALT_DATA_FORMAT
    addJsonVal(rslt,"alt_data_format",String(1),false,true,true);
#endif

    // two hardwired uint32_t columns
    // one for regular runs, and one for errors

	rslt += "\"col\":[\n";
        rslt += "{";
        addJsonVal(rslt,"name","runs",true,true,false);
        addJsonVal(rslt,"type","uint32_t",true,true,false);
        addJsonVal(rslt,"tick_interval",String(10),	false,false,true);
        rslt += "},\n";

        rslt += "{";
        addJsonVal(rslt,"name","errors",true,true,false);
        addJsonVal(rslt,"type","uint32_t",true,true,false);
        addJsonVal(rslt,"tick_interval",String(10),	false,false,true);
        rslt += "}\n";
	rslt += "]\n";

	rslt += "}";

	#if 0
		Serial.print("getBilgeChartHeader()=");
		Serial.println(rslt.c_str());
	#endif

	return rslt;
}





#if ALT_DATA_FORMAT
    typedef struct __attribute__ ((packed)) 
    {
        uint8_t col_idx;
        uint8_t err_idx;    // only for the error series
        uint32_t dt;
        uint32_t val;
    } outRecord_t;

    bool sendAltRecord(SDBackwards_t *iter,uint8_t col_idx, uint32_t dt, uint32_t val, uint8_t err_idx=0)
    {
        outRecord_t rec;
        rec.col_idx = col_idx;
        rec.err_idx = err_idx;
        rec.dt = dt;
        rec.val = val;
        if (!myiot_web_server->writeBinaryData((const char*)&rec, sizeof(outRecord_t)))
        {
            if (iter->file)
                iter->file.close();
            return false;
        }
        return true;
    }

#else
    typedef struct
    {
        uint32_t dt0;       // the zero point one second before the run
        uint32_t run0;
        uint32_t err0;

        uint32_t dt1;       // the start of the run
        uint32_t run1;      // a function of the duration, currently just assigned
        uint32_t err1;      // the error level of the run, if any

        uint32_t dt2;       // the end of the run, back to zero
        uint32_t run2;
        uint32_t err2;
    } outRecord_t;
#endif


String baHistory::sendBilgeChartData(uint32_t secs)
    // iterate in backwards chunks, and for each run generate
    // three data points:
    //
    //      ts-1/0              to start the run,
    //      ts/error_level      for the start
    //      ts+dur/0            for the end.
    //
    // The input dataLog records are 12 bytes and the output
    // records are 24 bytes each, and we send each one individually.
{
	uint32_t cutoff = secs ? time(NULL) - secs : 0;

	#if DEBUG_SEND_DATA
		String dbg_tm = timeToString(cutoff);
		LOGI("sendBilgeChartData(%d) since %s from %s",secs,secs?dbg_tm.c_str():"forever",history_filename);
	#endif

    SDBackwards_t iter;
    uint8_t stack_buffer[STACK_BUF_SIZE];
        // 516 holding 43 history records
    iter.chunked        = 1;
    iter.client_data    = cutoff;                      // cutoff 0 = all records
    iter.filename       = history_filename;
    iter.rec_size       = HIST_REC_SIZE;               // new 12 bytes
    iter.record_fxn     = baHistoryCondition;
    iter.buffer         = stack_buffer;                // an even multiple of rec_size
    iter.buf_size       = STACK_BUF_SIZE;
    iter.dbg_level      = 1;                           // 0..2

	if (!startSDBackwards(&iter))
		return "";

    if (!myiot_web_server->startBinaryResponse("application/octet-stream", CONTENT_LENGTH_UNKNOWN))
		return "";

	int num_file_recs = iter.file ? iter.file.size() / HIST_REC_SIZE : 0;
    #if DEBUG_SEND_DATA
        LOGI("num_file_recs(%d)",num_file_recs);
    #endif

	int sent = 0;
	int num_recs;
    #if ALT_DATA_FORMAT
        #if DEBUG_SEND_DATA > 1
            LOGD("sizeof(out_record)=%d",sizeof(outRecord_t));
        #endif
    #else
        outRecord_t out_record;
	#endif
    uint32_t *in_ptr = (uint32_t *) getSDBackwards(&iter,&num_recs);
	while (num_recs)
    {
        #if DEBUG_SEND_DATA > 1
            LOGD("processing num_recs(%d)",num_recs);
        #endif

        for (int i=0; i<num_recs; i++)
        {
            uint32_t dt = *in_ptr++;
            uint32_t dur = *in_ptr++;
            uint32_t flag = *in_ptr++;

            #if DEBUG_SEND_DATA > 1
                LOGD("    got(%d) run(%d,%s,%d,%d)",i,dt,timeToString(dt).c_str(),dur,flag);
            #endif

            #if ALT_DATA_FORMAT

                uint32_t err = 0;
                if (flag & HIST_STATE_EMERGENCY)
                    err = 5;
                else if (flag & HIST_STATE_CRITICAL_TOO_LONG)
                    err = 4;
                else if (flag & HIST_STATE_TOO_LONG)
                    err = 3;
                else if (flag & HIST_STATE_TOO_OFTEN_DAY)
                    err = 2;
                else if (flag & HIST_STATE_TOO_OFTEN_HOUR)
                    err = 1;

                // with suitable jury rigging of iotChart.js series options
                // this will put a marker on each run that has an error.
                // Now I would like to figure out how to mouse over the
                // marker and display the type of error, perhaps in a third series?

                if (!sendAltRecord(&iter,0,dt-1,0) ||
                    !sendAltRecord(&iter,0,dt,dur) ||
                    !sendAltRecord(&iter,0,dt+dur,0))
                    return "";

                if (flag && !sendAltRecord(&iter,1,dt,dur,err))
                    return "";

            #else

                uint32_t err = 0;
                if (flag & HIST_STATE_EMERGENCY)
                    err = 50;
                else if (flag & HIST_STATE_CRITICAL_TOO_LONG)
                    err = 40;
                else if (flag & HIST_STATE_TOO_LONG)
                    err = 30;
                else if (flag & HIST_STATE_TOO_OFTEN_DAY)
                    err = 20;
                else if (flag & HIST_STATE_TOO_OFTEN_HOUR)
                    err = 10;

                memset(&out_record, 0, sizeof(out_record));
                out_record.dt0 = dt-1;      // starting zero point
                out_record.dt1 = dt;        // the point
                out_record.run1 = dur;
                out_record.err1 = err;
                out_record.dt2 = dt + dur;  // ending zero point

                if (!myiot_web_server->writeBinaryData((const char*)&out_record, sizeof(out_record)))
                {
                    if (iter.file)
                        iter.file.close();
                    return "";
                }
            #endif

            sent ++;
        }
		in_ptr = (uint32_t *) getSDBackwards(&iter,&num_recs);
	}

	#if DEBUG_SEND_DATA
		LOGD("    sendBilgeChartData() sent %d/%d records",sent,num_file_recs);
	#endif

	return RESPONSE_HANDLED;
}



