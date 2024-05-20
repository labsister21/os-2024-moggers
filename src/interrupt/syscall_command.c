#include "syscall_command.h"
#include "framebuffer/framebuffer.h"
#include "keyboard/keyboard.h"
#include "process/process.h"

int8_t putchar(char* buffer, uint32_t text_color, CP* cursor_position){
    framebuffer_write((*cursor_position).row, (*cursor_position).col, *buffer, text_color, 0x0);
    if((*cursor_position).col >= VGA_WIDTH -1){
        (*cursor_position).row++;
        (*cursor_position).col = -1;
    }
    return 0;
}

int8_t puts(char* buffer, putsExtraAttribute* p, CP* cursor_position){
    for(int32_t i=0; i<(*p).count; i++){
        putchar(buffer+i, (*p).text_color, cursor_position);
        (*cursor_position).col++;
    }
    return 0;
}

int8_t move_text_cursor(uint16_t row, uint16_t col, uint32_t cursor_color){
    framebuffer_set_cursor(row, col);
    framebuffer_write(row, col, ' ', cursor_color, 0x0);
    return 0;
}

int8_t clear_screen() {
    framebuffer_clear();
    return 0;
}

uint16_t get_cursor_position(){
    return framebuffer_get_cursor_position();
}

uint8_t list_all_process(CP* cursor_position){
    for(uint32_t i=0; i<PROCESS_COUNT_MAX; i++){
        if(process_manager_state._process_used[i]){
            char buff[100];
            for(uint32_t j=0; j<30; j++){
                buff[j] = ' ';
            }

            putsExtraAttribute p = {
                .count = 13,
                .text_color = 0xf
            };

            uint32_t pid = _process_list[i].metadata.pid;
            uint32_t j = 3;
            if(pid == 0){
                buff[j] = '0';
            }
            while(pid != 0){
                buff[j] = (pid % 10) + (uint32_t)'0';
                pid /= 10;
            }

            memcpy(&buff[5], _process_list[i].metadata.name, 8);
            for(uint32_t j=5; j < 14; j++){
                if(buff[j] == 0x0){
                    buff[j] = ' ';
                }
            }
            if(_process_list[i].metadata.state == PROCESS_RUNNING){
                memcpy(&buff[13], "PROCESS_RUNNING", 16);
                p.count += 16;
            } else if(_process_list[i].metadata.state == PROCESS_READY){
                memcpy(&buff[13], "PROCESS_READY", 14);
                p.count += 14;
            } else if(_process_list[i].metadata.state == PROCESS_TERMINATED){
                memcpy(&buff[13], "PROCESS_TERMINATED", 19);
                p.count += 19;
            }

            puts(buff, &p, cursor_position);
            cursor_position->row++;
            cursor_position->col = 0;
        }
    }
}