//------------------------------------
// baExterns.h
//------------------------------------
#pragma once

// external declarations in various files

extern void startAlarm();
    // in baAlarm.cpp


//------------------------------------
// in baRuns.cpp
//------------------------------------

#define TEST_TIMES  1
    // define allows me to test in compressed time where
    // one hour == 1 minute, one day=3 minutes, and one week==5 minutes

#define DEBUG_RUNS  0
    // debugging in bilgeAlarm.cpp and baRuns.cpp


#define MAX_RUN_HISTORY    256      // 256 * 8 = 2K
    // set to smaller number (i.e 15) for testing circular buffer
    //
    // circular buffer of run history, where each history element
    // is 8 bytes (two 32 bit numbers), the first being the time
    // of the run, and the second being the duration of the run in seconds.
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


extern void initRunIterator(int *iterator);
extern runHistory_t *getNextRun(int *iterator);
extern runHistory_t *getPrevRun(int *iterator);

extern void startRun();
extern void setRunFlags(uint16_t flags);
extern uint32_t endRun();
extern void clearRuns();

extern uint32_t getStartDuration();
extern void updateStartDuration();

extern int countRuns(int type);
extern void setRunWindow(int type, int num);
extern String historyHTML();
