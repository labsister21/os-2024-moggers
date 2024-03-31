#include <stdint.h>

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
int8_t putchar(char* buffer, uint32_t text_color);

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
int8_t puts(char* buffer, uint32_t count, uint32_t text_color);