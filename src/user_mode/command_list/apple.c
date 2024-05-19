#include "apple.h"

void apple(Arg* argument){
    char buf[0x197570];
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) &buf,
        .name                  = "apple",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x197570,
    };
    // read file
    syscall(1, (uint32_t) &request, 0, 0);

    uint32_t frame_total = 2181;
    uint32_t frame_width = 50;
    uint32_t frame_height = 15;

    

    putsExtraAttribute p = {
        .count = 50,
        .text_color = BIOS_WHITE
    };

    for(uint32_t frame=0; frame < 2181; frame++){
        // clear screen
        syscall(9, 0, 0, 0);
        cursor_position.col = 0;
        cursor_position.row = 0;

        for(uint32_t row = 0; row < frame_height; row++){
            char temp[50];
            for(uint32_t col = 0; col < frame_width; col++){
                temp[col] = buf[(row + col) * frame];
            }
            syscall(6, (uint32_t) buf, (uint32_t) &p, (uint32_t) &cursor_position);
            cursor_position.row++;
        }
    }
}