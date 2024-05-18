#include <stdint.h>
#include <stdbool.h>

#include "std/string.h"

void syscall2(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

typedef struct {
    int32_t row;
    int32_t col;
} CP;

typedef struct {
    uint32_t count;
    uint32_t text_color;
} putsExtraAttribute;

struct Time {
    uint32_t second;
    uint32_t minute;
    uint32_t hour;
    uint32_t day;
    uint32_t month;
    uint32_t year;
};

#define DEFAULT_ROW 24
#define DEFAULT_COL 60

int main(void){
    CP cursor_position = {
        .row = DEFAULT_ROW,
        .col = DEFAULT_COL,
    };

    putsExtraAttribute p = {
        .count = 19,
        .text_color = 0xE
    };

    struct Time current_time = {

    };

    while(true){
        // reset time
        memset(&current_time, 0x0, sizeof(struct Time));
        syscall2(14, (uint32_t) &current_time, 0, 0);

        // parse to char
        char buf[19];

        buf[0] = current_time.year / 1000;
        buf[1] = (current_time.year % 1000) / 100;
        buf[2] = (current_time.year % 100) / 10;
        buf[3] = (current_time.year % 10);
        buf[4] = ' ';
        buf[5] = (current_time.month / 10);
        buf[6] = current_time.month % 10;
        buf[7] = ' ';
        buf[8] = (current_time.day / 10);
        buf[9] = current_time.day % 10;
        buf[10] = ' ';
        buf[11] = (current_time.hour / 10);
        buf[12] = current_time.hour % 10;
        buf[13] = ':';
        buf[14] = (current_time.minute / 10);
        buf[15] = current_time.minute % 10;
        buf[16] = ':';
        buf[17] = (current_time.second / 10);
        buf[18] = current_time.second % 10;

        for(uint32_t i=0; i<19; i++){
            if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16){
                continue;
            }
            else {
                buf[i] = buf[i] + (uint32_t) '0';
            }
        }

        syscall2(6, (uint32_t) buf, (uint32_t) &p, (uint32_t) &cursor_position);

        cursor_position.col = DEFAULT_COL;
        cursor_position.row = DEFAULT_ROW;
    }
    return 0;
}