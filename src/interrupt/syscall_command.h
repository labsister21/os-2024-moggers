#include <stdint.h>

/**
 * Struct to store cursor position from USER-SHELL.
 * Does not store the value.
*/
typedef struct {
    int32_t row;
    int32_t col;
} CP;

typedef struct {
    uint32_t count;
    uint32_t text_color;
} putsExtraAttribute;

/**
 * write to screen (puts) function.
 * 
 * `eax: 5`
 * 
 * @var eax6
 * @param ebx `buffer`      pointer to char buffer
 * @param ecx `text_color`  text color
 * @return uint8_t      return code
 */
int8_t putchar(char* buffer, uint32_t text_color, CP* cursor_position);

/**
 * write to screen (puts) function.
 * 
 * `eax: 6`
 * 
 * @var eax6
 * @param ebx `buffer`      pointer to buffer
 * @param ecx `count`       char count
 * @param edx `text_color`  text color
 * @return uint8_t      return code
 */
int8_t puts(char* buffer, putsExtraAttribute* p, CP* cursor_position);

/**
 * move text cursor.
 * 
 * `eax: 8`
 * 
 * @var eax6
 * @param ebx `r`       int row
 * @param ecx `c`       int column
 * @return uint8_t      return code
 */
int8_t move_text_cursor(uint16_t r, uint16_t c, uint32_t cursor_color);

/**
 * clear screen.
 * 
 * `eax: 9`
 * 
 * @var eax6
 * @return uint8_t      return code
 */
int8_t clear_screen();

uint16_t get_cursor_position();