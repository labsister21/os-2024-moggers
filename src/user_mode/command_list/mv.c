#include "mv.h"

void mv(Arg* argument1, Arg* argument2){
    uint32_t cluster_needed = current_dir.dir_table_buf.table[get_entry_row(argument1)].filesize / CLUSTER_SIZE;
    int mod = current_dir.dir_table_buf.table[get_entry_row(argument1)].filesize % CLUSTER_SIZE;
    if(mod != 0) cluster_needed++;

    struct ClusterBuffer cl[cluster_needed];
    for(uint32_t i=0; i<cluster_needed; i++){
        for(int j=0; j<CLUSTER_SIZE; j++){
            cl[i].buf[j] = 0x0;
        }
    }
    // handlling argument 1 - source
    struct FAT32DriverRequest source = {
        .buf = &cl,
        .buffer_size = sizeof(current_dir.cluster_buf),
        .parent_cluster_number = get_cluster_number()
    };
    memcpy(source.name, (*argument1).info, (*argument1).len_info);

    uint32_t entry_row = get_entry_row(argument2);

    if(entry_row == -1){
        // rename file
        uint8_t res_code;
        // read file
        syscall(0, (uint32_t) &source, (uint32_t) &res_code, 0);
        // delete file
        syscall(3, (uint32_t) &source, (uint32_t) &res_code, 0);
        // copy filename with new name
        memcpy(source.name, (*argument2).info, 8);
        // write file with new filename
        syscall(2, (uint32_t) &source, (uint32_t) &res_code, 0);
    } else if(current_dir.dir_table_buf.table[entry_row].attribute == ATTR_SUBDIRECTORY) {
        // move file into folder
        uint8_t res_code;
        // read file
        syscall(0, (uint32_t) &source, (uint32_t) &res_code, 0);
        // delete file
        syscall(3, (uint32_t) &source, (uint32_t) &res_code, 0);
        // write file in new destination
        source.parent_cluster_number = get_cluster_number_self(&current_dir.dir_table_buf.table[entry_row]);
        syscall(2, (uint32_t) &source, (uint32_t) &res_code, 0);
    }
}