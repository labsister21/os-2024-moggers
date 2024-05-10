#include "rm.h"

void rm(Arg* argument){
    struct FAT32DriverRequest request = {
        .buffer_size = 0,
        .parent_cluster_number = get_cluster_number(),
    };
    memcpy(request.name, (*argument).info, (*argument).len_info);
    memcpy(request.ext, (*argument).info, (*argument).len_ext);

    uint8_t res_code;
    syscall(3, (uint32_t) &request, (uint32_t) &res_code, 0);
}