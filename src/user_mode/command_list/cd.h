#ifndef _COMMAND_CD
#define _COMMAND_CD

#include "user_mode/user-shell.h"
#include "filesystem/fat32.h"

void cd(Arg* argument);

void cd_root();

void cd_refresh();

#endif