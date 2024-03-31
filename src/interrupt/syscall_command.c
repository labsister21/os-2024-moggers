#include "syscall_command.h"
#include "framebuffer/framebuffer.h"
#include "keyboard/keyboard.h"

int8_t putchar(char* buffer, uint32_t text_color){
    framebuffer_write(0, 0, *buffer, text_color, 0x0);
    return 0;
}

int8_t puts(char* buffer, uint32_t count, uint32_t text_color){
    for(int32_t i=0; i<count; i++){
        framebuffer_write(0, i, buffer[i], text_color, 0x0);
    }
    return 0;
}