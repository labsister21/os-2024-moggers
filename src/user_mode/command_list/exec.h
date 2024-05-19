#ifndef _COMMAND_EXEC
#define _COMMAND_EXEC

#include "user_mode/user-shell.h"
#include "filesystem/fat32.h"
#include "process/process.h"

void exec(Arg* argument);

#endif