#include "find.h"

void find(Arg* argument){
    CurrentWorkingDirectory path_result;
    memcpy(&path_result, &current_working_directory, sizeof(current_working_directory));

    find_recursive(path_result, current_dir.dir_table_buf, *argument, &path_result);

    char str[2048];
    uint32_t j = 0;
    for(uint32_t i=0; i<path_result.neff; i++){
        uint32_t k = 0;
        while(path_result.path[i].info[k] != 0x0){
            str[j] = path_result.path[i].info[k];
            j++;
            k++;
        }
        str[j] = '/';
        j++;
    }
    uint32_t k = 0;
    while(argument->info[k] != 0x0){
        str[j] = argument->info[k];
        k++;
        j++;
    }

    // write the file
    putsExtraAttribute p = {
        .count = j,
        .text_color = BIOS_WHITE
    };

    syscall(6, (uint32_t) &str, (uint32_t) &p, (uint32_t) &cursor_position);

    // cleanup
    cursor_position.col = 0;
    cursor_position.row++;

}

bool find_recursive(CurrentWorkingDirectory path_result, struct FAT32DirectoryTable dtable, Arg end, CurrentWorkingDirectory *ans_path){
    uint32_t entry_size = CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry);

    for(uint32_t i=2; i<entry_size; i++){
        if(dtable.table[i].user_attribute == UATTR_NOT_EMPTY){
            CurrentWorkingDirectory to_passed = path_result;
            if(!memcmp(dtable.table[i].name, end.info, 8) && !memcmp(dtable.table[i].ext, end.ext, 3)){
                memcpy(ans_path, &to_passed, sizeof(to_passed));
                return true;
            }
            else if(dtable.table[i].attribute == ATTR_SUBDIRECTORY){
                // is folder
                struct FAT32DirectoryTable buffer;
                struct FAT32DriverRequest request = {
                    .buf = &buffer,
                    .buffer_size = sizeof(buffer),
                    .parent_cluster_number = get_cluster_number_self(&dtable.table[i]),
                };

                memcpy(request.name, dtable.table[i].name, 8);
                uint32_t response_code;
                syscall(1, (uint32_t) &request, (uint32_t) &response_code, 0);
                
                Arg temp_arg = {
                    .len_ext = 3,
                    .len_info = 8,
                };

                memcpy(temp_arg.info, dtable.table[i].name, 8);
                memcpy(temp_arg.ext, dtable.table[i].ext, 3);

                to_passed.path[to_passed.neff] = temp_arg;
                to_passed.neff++;
                bool res = find_recursive(to_passed, buffer, end, ans_path);
                if(res){
                    return true;
                }
            } 
        }
    }
    return false;
}