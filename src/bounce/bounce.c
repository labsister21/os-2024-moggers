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
#define DEFAULT_COL 0

int main(void){
    CP cursor_position = {
        .row = DEFAULT_ROW,
        .col = DEFAULT_COL,
    };

    CP previous_cursor = {
        .row = 0,
        .col = 0
    };

    putsExtraAttribute p = {
        .count = 1,
        .text_color = 0xA
    };

    struct Time current_time = {

    };

    uint32_t prev_time = 0;

    while(true){

        // reset time
        memset(&current_time, 0x0, sizeof(struct Time));
        syscall2(14, (uint32_t) &current_time, 0, 0);

        if(current_time.second != prev_time){
            syscall2(5, (uint32_t) 0x0, 0x0, (uint32_t) &previous_cursor);
            prev_time = current_time.second;

            char buf = 0x0;
            previous_cursor.col = cursor_position.col;

            syscall2(6, (uint32_t) buf, (uint32_t) &p, (uint32_t) &cursor_position);

            previous_cursor.row = cursor_position.row;

            if(cursor_position.col == 79){
                cursor_position.col = 0;
            }
        }
    }
    return 0;
}