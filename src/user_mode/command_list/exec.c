#include "exec.h"

void exec(Arg* argument){
    
    syscall(15, 0, 0, 0);
}