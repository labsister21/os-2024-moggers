#include "cat.h"

void cat(Arg* argument){
    uint32_t cluster_needed = current_dir.dir_table_buf.table[get_entry_row(argument)].filesize / CLUSTER_SIZE;
    int mod = current_dir.dir_table_buf.table[get_entry_row(argument)].filesize % CLUSTER_SIZE;
    if(mod != 0) cluster_needed++;

    struct ClusterBuffer cl[cluster_needed];
    for(uint32_t i=0; i<cluster_needed; i++){
        for(int j=0; j<CLUSTER_SIZE; j++){
            cl[i].buf[j] = 0x0;
        }
    }

    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .parent_cluster_number = get_cluster_number(),
        .buffer_size           = sizeof(cl),
    };
    // set name
    uint8_t test = (*argument).len_info;
    memcpy(request.name, (*argument).info, (*argument).len_info);
    memcpy(request.ext, (*argument).ext, (*argument).len_ext);

    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);

    if(!retcode){
        uint32_t size = 0;
        uint32_t i = 0;
        char buf = cl[0].buf[i];
        while(buf != 0x0){
            if(buf == '\n'){
                cursor_position.col = 0;
                cursor_position.row++;
            } else {
                syscall(5, (uint32_t) &buf, BIOS_WHITE, (uint32_t) &cursor_position);
                cursor_position.col++;
            }
            i++;
            buf = cl[0].buf[i];
        }
    }

    // cleanup
    cursor_position.col = 0;
    cursor_position.row++;
}