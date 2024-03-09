#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "framebuffer.h"
#include "std/string.h"
#include "portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    // TODO : Implement
    uint8_t pos = r * 80 + c;
 
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    // TODO : Implement
    uint16_t pos = (row * (80 * 2)) + col * 2;
    uint16_t colourBit = (bg << 4) | (fg & 0xF);

    FRAMEBUFFER_MEMORY_OFFSET[pos] = c;
    FRAMEBUFFER_MEMORY_OFFSET[pos+1] = colourBit;
}

void framebuffer_clear(void) {
    // TODO : Implement
    memset(FRAMEBUFFER_MEMORY_OFFSET, 0x00, 25 * 2 * 80 * 2);
}