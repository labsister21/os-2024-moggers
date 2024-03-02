#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "framebuffer.h"
#include "portio.h"
#include "std/string.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    // TODO : Implement
    uint8_t pos = r * 80 + c;
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, ((pos >> 8) & 0x00FF));
    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, pos & 0x00FF);
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    // TODO : Implement

    uint16_t colourBit = (bg << 4) | fg ;
    uint16_t fullBit = (colourBit << 8) | c ;

    // Assign the character 2 byte into the memory
    volatile uint16_t* where = (volatile uint16_t*) FRAMEBUFFER_MEMORY_OFFSET + (row*80 + col) ;
    *where = fullBit;
}

void framebuffer_clear(void) {
    // TODO : Implement
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            framebuffer_write(i, j, 0x00, 0x7, 0);
        }
    }
}