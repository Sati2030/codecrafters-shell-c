#ifndef _PROG_INTERFACING_H
#define _PROG_INTERFACING_H
#include "argumenter.h"
#include "input_reader.h"

    void program_execution(Arguments *args);
    char *valid_command(char *input);
    char *get_path();
    Arguments get_matches(char *input);

#endif