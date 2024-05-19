#include "apple.h"

void apple(Arg* argument){
    char buf[0x8a713];
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) &buf,
        .name                  = "os-dev",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x8a713,
    };
    // read file
   uint32_t res;
   syscall(0, (uint32_t) &request, &res, 0);

    uint32_t frame_total = 2181;
    uint32_t frame_width = 26;
    uint32_t frame_height = 10;

    

    

    for(uint32_t frame=0; frame < frame_total; frame++){
        // clear screen
        syscall(9, 0, 0, 0);
        
        
        cursor_position.col = 0;
        cursor_position.row = 0;

        for(uint32_t row = 0; row < frame_height; row++){
            char temp[frame_width];


            for(uint32_t col = 0; col < frame_width; col++){
                temp[col] = buf[((row * frame_width) + col) + frame * (frame_height * frame_width)];
            }
            
            // temp[frame_width] = ' ';
            temp[frame_width-1] = ' ';

            putsExtraAttribute p = {
                .count = frame_width,
                .text_color = BIOS_WHITE
            };

            syscall(6, (uint32_t) &temp, (uint32_t) &p, (uint32_t) &cursor_position);
            cursor_position.row++;
            cursor_position.col = 0;
        }

        uint32_t i = 0xFFFFFF;
        while(i){
            i--;
        }
    }
}