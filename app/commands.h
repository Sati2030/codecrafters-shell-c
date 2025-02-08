#ifndef _COMMANDS_H
#define _COMMANDS_H
#include "argumenter.h"
#include "prog_interfacing.h"

    void command_handling(Arguments *args);
    void echo(Arguments *args);
    void type(Arguments *args);
    int valid_command(char *arg);
    void exit_(Arguments *args);
    void pwd();
    void cd (char *dir);

#endif