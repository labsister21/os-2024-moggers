#include "kill.h"

void kill(Arg* argument){
    uint32_t pid = 0;

    for(uint32_t i=0; i<argument->len_info; i++){
        pid = pid * 10 + argument->info[i] - '0';
    }

    syscall(13, pid, 0, 0);
}