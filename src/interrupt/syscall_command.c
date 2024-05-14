#include "syscall_command.h"
#include "framebuffer/framebuffer.h"
#include "keyboard/keyboard.h"

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