#include "cmos.h"

int century_register = 0x00;

int get_update_in_progress_flag() {
      out(CMOS_ADDRESS, 0x0A);
      return (in(CMOS_DATA) & 0x80);
}
 
unsigned char get_RTC_register(int reg) {
      out(CMOS_ADDRESS, reg);
      return in(CMOS_DATA);
}
 
void read_rtc(struct Time* current_time) {
      // local variable to ensure the validity and persistence of date and time gotten
      uint32_t century;
      uint32_t last_second;
      uint32_t last_minute;
      uint32_t last_hour;
      uint32_t last_day;
      uint32_t last_month;
      uint32_t last_year;
      uint32_t last_century;
      uint32_t registerB;
 
      while (get_update_in_progress_flag());     
      uint32_t second = get_RTC_register(0x00);
      uint32_t minute = get_RTC_register(0x02);
      uint32_t hour = get_RTC_register(0x04);
      uint32_t day = get_RTC_register(0x07);
      uint32_t month = get_RTC_register(0x08);
      uint32_t year = get_RTC_register(0x09);

      if(century_register != 0) {
            century = get_RTC_register(century_register);
      }
 
      do {
            last_second = second;
            last_minute = minute;
            last_hour = hour;
            last_day = day;
            last_month = month;
            last_year = year;
            last_century = century;
 
            while (get_update_in_progress_flag());  
            second = get_RTC_register(0x00);
            minute = get_RTC_register(0x02);
            hour = get_RTC_register(0x04);
            day = get_RTC_register(0x07);
            month = get_RTC_register(0x08);
            year = get_RTC_register(0x09);

            if(century_register != 0) {
                  century = get_RTC_register(century_register);
            }

      } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
               (last_day != day) || (last_month != month) || (last_year != year) ||
               (last_century != century) );
 
      registerB = get_RTC_register(0x0B);
 
      // Convert BCD to binary values
 
      if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = (( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) + 7 ) % 24)  | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10) + !!(hour % 24);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(century_register != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
 
      // Convert 12 hour clock to 24 hour clock and add 7 to synchronize with WIB
      if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      }
 
      // Calculate the full (4-digit) year
      if(century_register != 0) {
            year += century * 100;
      } else {
            year += (CURRENT_YEAR / 100) * 100;
            if(year < CURRENT_YEAR) year += 100;
      }


      current_time->day = day;
      current_time->hour = hour;
      current_time->minute = minute;
      current_time->month = month;
      current_time->second = second;
      current_time->year = year;
}