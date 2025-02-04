#ifndef _PROG_INTERFACING_H
#define _PROG_INTERFACING_H
#include "argumenter.h"
#include "input_reader.h"

    typedef struct{
        char **entries;
        int count;
    } Entries;

    void program_execution(Arguments *args);
    char *valid_command(char *input);
    char *get_path();
    Entries get_matches(char *input);

#endif