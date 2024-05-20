#include "keyboard.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/portio.h"
#include "std/string.h"

struct KeyboardDriverState keyboard_state = {
    .keyboard_input_on = false,
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_state_activate(void){
    if(!keyboard_state.keyboard_input_on){
        keyboard_state.keyboard_input_on = true;
    }
}

void keyboard_state_deactivate(void){
    if(keyboard_state.keyboard_input_on){
        keyboard_state.keyboard_input_on = false;
    }
}

void get_keyboard_buffer(char *buf){
    char c = keyboard_state.keyboard_buffer;
    *buf = c;
    keyboard_state.keyboard_buffer = 0x0;
}


void keyboard_isr(void) {

    uint16_t scancode = in(KEYBOARD_DATA_PORT);
    // TODO : Implement scancode processing
    if(keyboard_state.keyboard_input_on){
        uint16_t mapped = keyboard_scancode_1_to_ascii_map[scancode];
        keyboard_state.keyboard_buffer = mapped;
    }
    pic_ack(PIC1_OFFSET + IRQ_KEYBOARD);
}

/*
if(c == '\n'){
    keyboard_state.row++;
    keyboard_state.col = 0;
    framebuffer_set_cursor(keyboard_state.row, keyboard_state.col);
    framebuffer_write(keyboard_state.row, keyboard_state.col, '\0', 0xF, 0x0);
} else if(c == '\t') {
    uint8_t tab_size = 4;
    for(uint8_t i=0; i<tab_size; i++){
        framebuffer_write(keyboard_state.row, keyboard_state.col++, '\0', 0xF, 0x0);
        framebuffer_set_cursor(keyboard_state.row, keyboard_state.col);
        framebuffer_write(keyboard_state.row, keyboard_state.col, '\0', 0xF, 0x0);
    }
} else if(c == '\b'){
    keyboard_state.col--;
    framebuffer_set_cursor(keyboard_state.row, keyboard_state.col);
    framebuffer_write(keyboard_state.row, keyboard_state.col, '\0', 0xF, 0x0);
} 
else if(c != 0x00){
    framebuffer_write(keyboard_state.row, keyboard_state.col++, c, 0xF, 0x0);
    framebuffer_set_cursor(keyboard_state.row, keyboard_state.col);
    framebuffer_write(keyboard_state.row, keyboard_state.col, '\0', 0xF, 0x0);
}
*/