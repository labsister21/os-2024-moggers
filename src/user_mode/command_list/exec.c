#include "exec.h"

void exec(Arg* argument){
    
    syscall(15, (uint32_t)&(*argument).info, get_cluster_number(), 0);
}