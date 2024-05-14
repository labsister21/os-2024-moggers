#ifndef _COMMAND_FIND
#define _COMMAND_FIND

#include "user_mode/user-shell.h"
#include "filesystem/fat32.h"

void find(Arg* argument);

bool find_recursive(CurrentWorkingDirectory path_result, struct FAT32DirectoryTable dtable, Arg end, CurrentWorkingDirectory *ans_path);

#endif