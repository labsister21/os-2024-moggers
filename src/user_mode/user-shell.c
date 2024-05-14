#include <stdint.h>
#include "filesystem/fat32.h"
#include "std/string.h"
#include "user-shell.h"

#include "command_list/cat.h"
#include "command_list/ls.h"
#include "command_list/mkdir.h"
#include "command_list/cd.h"
#include "command_list/clear.h"
#include "command_list/help.h"
#include "command_list/mv.h"
#include "command_list/rm.h"
#include "command_list/find.h"

/* DEFINE STRUCT TO BE USED */
CP cursor_position = {
    .row = 0,
    .col = 0
};

struct FAT32DriverState current_dir = {

};

CurrentWorkingDirectory current_working_directory = {
    .neff = 0,
    .path = {0}
};
#include "std/string.h"
#include "user-shell.h"

#include "command_list/cat.h"
#include "command_list/ls.h"
#include "command_list/mkdir.h"
#include "command_list/cd.h"
#include "command_list/clear.h"
#include "command_list/help.h"
#include "command_list/mv.h"
#include "command_list/rm.h"
#include "command_list/find.h"

/* DEFINE STRUCT TO BE USED */
CP cursor_position = {
    .row = 0,
    .col = 0
};

struct FAT32DriverState current_dir = {

};

CurrentWorkingDirectory current_working_directory = {
    .neff = 0,
    .path = {0}
};

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

/* FILE SYSTEM HELPER */

uint32_t get_entry_row(Arg* argument){
    uint32_t entry_size = CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry);
    for(uint32_t i=2; i<entry_size; i++){
        if(memcmp(current_dir.dir_table_buf.table[i].name, (*argument).info, 8) == 0 && // check name
           memcmp(current_dir.dir_table_buf.table[i].ext, (*argument).ext, 3) == 0){ // check ext
            return i;
        }
    }
    return -1;
}

uint32_t get_cluster_number(){
    uint16_t cluster_high = current_dir.dir_table_buf.table[0].cluster_high;
    uint16_t cluster_low = current_dir.dir_table_buf.table[0].cluster_low;
    uint32_t cluster_num = (cluster_high << 16) + cluster_low;

    return cluster_num;
}

uint32_t get_cluster_number_self(struct FAT32DirectoryEntry *p){
    uint16_t cluster_high = (*p).cluster_high;
    uint16_t cluster_low = (*p).cluster_low;
    uint32_t cluster_num = (cluster_high << 16) + cluster_low;

    return cluster_num;
}

/* BUILT IN FUNCTIONS TO HELP */

void handle_keyboard_input(char *buf){
    if(*buf == '\b'){
        if (cursor_position.col > 3){
            cursor_position.col--;
            syscall(5, (uint32_t) 0x0, BIOS_WHITE, (uint32_t) &cursor_position);
        }
    } else {
        syscall(5, (uint32_t) buf, BIOS_WHITE, (uint32_t) &cursor_position);
        cursor_position.col++;
    }
    syscall(8, cursor_position.row, cursor_position.col, BIOS_WHITE);
}

void get_command(CommandStruct *command, Arg *args_list){
    char buf;

    while(buf != '\n'){
        syscall(4, (uint32_t) &buf, 0, 0);

        if(buf != 0x0 && buf != '\n'){
            handle_keyboard_input(&buf);

            if(buf == '\b'){
                if(command->len_info > 0){
                    command->info[command->len_info] = 0x0;
                    command->len_info--;
                }
            }
            else if(buf == ' '){
                command->info[command->len_info] = ' ';
                command->len_info++;
            }
            else {
                command->info[command->len_info] = buf;
                command->len_info++;
            }
        }
    }

    buf = 0x0;

    // parsing
    uint32_t first_space_location = -1;
    bool isFirst = false;

    uint32_t i = 0;
    uint32_t first_args = -1;
    bool isArgs = false;
    while(i < command->len_info){
        isArgs = false;
        // find first space
        while(i < command->len_info){
            buf = command->info[i];
            if(buf == ' '){
                if(!isFirst){
                    first_space_location = i;
                    isFirst = true;
                }
                break;
            }
            i++;
        }

        // find first letter of args
        while(buf == ' ' && i < command->len_info){
            isArgs = true;
            buf = command->info[i];
            if(buf != ' '){
                first_args = i;
                break;
            }
            i++;
        }

        // parse args
        while(buf != ' ' && buf != 0x0 && i < command->len_info){
            uint32_t *idx = &args_list[command->argument_neff].len_info;
            args_list[command->argument_neff].info[*idx] = buf;
            (*idx)++;
            i++;
            buf = command->info[i];
        }

        if(isArgs){
            command->argument_neff++;
        }
    }

    // clean up
    command->len_info = (first_space_location != -1) ? first_space_location : i;
    int temp_pos_row = cursor_position.row;
    int temp_pos_col = cursor_position.col;
    cursor_position.col = 0;
    cursor_position.row++;
}

void print_template(){
    putsExtraAttribute p = {
        .count = 17,
        .text_color = BIOS_LIGHT_GREEN
    };
    syscall(6, (uint32_t) "Moggers@OS-IF2230", (uint32_t) &p, (uint32_t) &cursor_position);


    char path[1000] ;
    int index = 0 ;
    for (int i = 0 ; i < current_working_directory.neff ; i++) {
        for (int j = 0 ; j < current_working_directory.path[i].len_info ; j++) {
            path[index] = current_working_directory.path[i].info[j] ;
            index += 1 ;
        }
        path[index] = '/' ;
        index += 1 ;
    }

    p.count = index ;
    p.text_color = BIOS_LIGHT_RED ;

    syscall(6, (uint32_t) path, (uint32_t) &p, (uint32_t) &cursor_position);

    p.count = 3;
    p.text_color = BIOS_LIGHT_GRAY;
    cursor_position.row++ ;
    cursor_position.col = 0 ;

    syscall(6, (uint32_t) ":$ ", (uint32_t) &p, (uint32_t) &cursor_position);

    // call move text cursor
    syscall(8, cursor_position.row, cursor_position.col, BIOS_WHITE);
}

void print_unknown(char* command, int size){
    char message[2048];
    int i=0;
    for(i=0; i<size; i++){
        message[i] = command[i];
    }

    putsExtraAttribute p = {
        .count = i,
        .text_color = BIOS_WHITE
    };

    syscall(6, (uint32_t) &message, (uint32_t) &p, (uint32_t) &cursor_position);

    char message2[] = " : command not found";
    p.count = sizeof(message2) / sizeof(message2[0]);
    p.text_color = BIOS_LIGHT_RED;

    for(int j = 0; j < p.count; j++){
        message[i] = message2[j];
        i++;
    }

    syscall(6, (uint32_t) &message2, (uint32_t) &p, (uint32_t) &cursor_position);

    cursor_position.col = 0;
    cursor_position.row++;
}

int main(void) {
    // get default current directory (root)
    cd_root();
    
    // struct FAT32DirectoryEntry *clb = current_dir.dir_table_buf.table;

    syscall(7, 0, 0, 0); // activate keyboard

    cursor_position.col = 0;
    cursor_position.row = 0;

    CommandStruct command;
    Arg args_list[DEFAULT_ARGS_COUNT];
    while(true){
        print_template();

        // clear command
        memset(&command, 0x0, sizeof(CommandStruct));
        memset(&args_list, 0x0, sizeof(args_list));

        get_command(&command, args_list);

        
        if(!memcmp(command.info, "cat", command.len_info)){
            if(command.argument_neff > 0){
                cat(&args_list[0]);
            }
        }
        else if(!memcmp(command.info, "ls", command.len_info)){
            ls(args_list);
        }
        else if(!memcmp(command.info, "mkdir", command.len_info)){
            mkdir(&args_list[0]);
            cd_refresh();
        }
        else if(!memcmp(command.info, "cd", command.len_info)){
            cd(&args_list[0]);
        }
        else if(!memcmp(command.info, "rm", command.len_info)){
            rm(&args_list[0]);
            cd_refresh();
        }
        else if(!memcmp(command.info, "mv", command.len_info)){
            mv(&args_list[0], &args_list[1]);
            cd_refresh();
        }
        else if(!memcmp(command.info, "find", command.len_info)){
            find(&args_list[0]);
        }
        else if(!memcmp(command.info, "clear", command.len_info)){
            clear();
        }
        else {
            print_unknown(command.info, command.len_info);
        }
    }
    
    // __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
    
    return 0;
}
