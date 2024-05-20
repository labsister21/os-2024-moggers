#ifndef _CMOS_H
#define _CMOS_H

#include "framebuffer/framebuffer.h"
#include "framebuffer/portio.h"
#include "std/string.h"

// macro to define current year
#define CURRENT_YEAR        2023

// macro of CMOS IO port
#define CMOS_ADDRESS        0x70
#define CMOS_DATA           0x71

// global variables needed to save the date and time readed from register
struct Time {
    uint32_t second;
    uint32_t minute;
    uint32_t hour;
    uint32_t day;
    uint32_t month;
    uint32_t year;
};

// return true if update in register is happening
int get_update_in_progress_flag();

// get the date time value from CMOS register 
unsigned char get_RTC_register(int reg);

// read the current time and update the global variables (second, minute, etc)
void read_rtc(struct Time*);

#endif