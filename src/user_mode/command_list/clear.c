#include "clear.h"

void clear(){
    syscall(9, 0, 0, 0);

    // clean up
    cursor_position.col = 0;
    cursor_position.row = 0;
}