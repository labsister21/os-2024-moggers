#include "ls.h"

void ls(Arg* argument){
    uint32_t i = 0;
    while(i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)){
        if(current_dir.dir_table_buf.table[i].user_attribute == UATTR_NOT_EMPTY){
            putsExtraAttribute p = {
                .count = 8,
                .text_color = BIOS_WHITE
            };

            if(current_dir.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                p.text_color = BIOS_LIGHT_BLUE;
            }

            syscall(6, (uint32_t) &current_dir.dir_table_buf.table[i].name, (uint32_t) &p, (uint32_t) &cursor_position);
            cursor_position.col = 0;
            cursor_position.row++;
        }
        i++;
    }
}