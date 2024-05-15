#include "cd.h"

void cd(Arg* argument){
    // putsExtraAttribute p = {
    //     .count = 5,
    //     .text_color = BIOS_LIGHT_GREEN
    // };
    struct FAT32DriverRequest request = {
        .buf = &current_dir.dir_table_buf.table,
        .buffer_size = sizeof(current_dir.cluster_buf),
        .parent_cluster_number = get_cluster_number()
    };

    // struct FAT32DriverState temp ;

    memset(&current_dir, 0x0, sizeof(struct FAT32DriverState));
    memcpy(request.name, (*argument).info, (*argument).len_info);
    uint32_t response_code;
    syscall(1, (uint32_t) &request, (uint32_t) &response_code, 0);

    // uint32_t i = 0;
    // bool found = false ;
    // while(i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) && !found){
    //     if(current_dir.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY){
    //         if(current_dir.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
    //             if (memcmp(&current_dir.dir_table_buf.table[i].name, (*argument).info, 8)) {
    //                 found = true ;
    //             }
    //         }
    //     }
    //     i++;
    // }
    // if (!found) {
    //     syscall(6, (uint32_t) "Gagal", (uint32_t) &p, (uint32_t) &cursor_position);
    //     return ;
    // }


    // update path
    if(!memcmp(argument->info, "..", 3)){
        char root[4] ;
        memcpy(&root, "root", 4) ;
        if (memcmp(current_working_directory.path[current_working_directory.neff-1].info, &root, 4)) {
            current_working_directory.neff--;
            memset(&current_working_directory.path[current_working_directory.neff], 0x0, sizeof(current_working_directory.path[current_working_directory.neff]));

            // memset(&current_dir, 0x0, sizeof(struct FAT32DriverState));
            // memcpy(request.name, (*argument).info, (*argument).len_info);
            // uint32_t response_code;
            // syscall(1, (uint32_t) &request, (uint32_t) &response_code, 0);
        }
    } else {
        uint32_t i = 0;
        // if (!memcmp(&current_working_directory.path[current_working_directory.neff-1].info, argument->info, 8)) {

        // }
        while(i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)){
            if(current_dir.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY){
                if(current_dir.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                    if ((memcmp(&current_dir.dir_table_buf.table[i].name, (*argument).info, 8))) {
                        current_working_directory.path[current_working_directory.neff].len_info = argument->len_info ;
                        memcpy(&current_working_directory.path[current_working_directory.neff], (*argument).info, 8);
                        current_working_directory.neff++;

                        // memset(&current_dir, 0x0, sizeof(struct FAT32DriverState));
                        // memcpy(request.name, (*argument).info, (*argument).len_info);
                        // uint32_t response_code;
                        // syscall(1, (uint32_t) &request, (uint32_t) &response_code, 0);
                        return ;
                    }
                    // if (i == CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)-1) {
                    //     memcpy(&current_dir, &temp, sizeof(struct FAT32DriverState));
                    // }
                }
            }
            i++;
        }
    }
}

void cd_root(){
    memset(&current_dir, 0x0, sizeof(struct FAT32DriverState));
    struct FAT32DriverRequest root_folder = {
        .buf = &current_dir.dir_table_buf.table,
        .name = "root",
        .buffer_size = sizeof(current_dir.cluster_buf),
        .parent_cluster_number = 2
    };
    uint32_t response_code;
    syscall(1, (uint32_t) &root_folder, (uint32_t) &response_code, 0);


    // update path
    current_working_directory.path[current_working_directory.neff].len_info = 4;
    memcpy(&current_working_directory.path[current_working_directory.neff], "root", 8);
    current_working_directory.neff++;
}

void cd_refresh(){
    char temp_name[8];
    memcpy(temp_name, current_dir.dir_table_buf.table[0].name, 8);

    struct FAT32DriverRequest request = {
        .buf = &current_dir.dir_table_buf.table,
        .buffer_size = sizeof(current_dir.cluster_buf),
        .parent_cluster_number = get_cluster_number()
    };
    memset(&current_dir, 0x0, sizeof(struct FAT32DriverState));
    memcpy(&request.name, temp_name, 8);
    uint32_t response_code;
    syscall(1, (uint32_t) &request, (uint32_t) &response_code, 0);
}