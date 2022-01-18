//------------------------------------
// baAlarm.h
//------------------------------------

#pragma once

extern void startAlarm();
extern void initPixels();
extern void setPixelBright(bool external,uint8_t val);
extern void showIncSetupPixel();
    // called 3 times by myIOTDevice and once when alarmTask() is running()
