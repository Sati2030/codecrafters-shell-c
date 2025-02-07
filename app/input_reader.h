#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "argumenter.h"
#include "prog_interfacing.h"

void deactivateCannonMode();
char *readInput();
char *other_tab(char *input,int *count);
char *complete_input(char *input, char *completion, int *count);
char *backspace(char *input,int *count);
void activateCannonMode();

#endif