#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "prog_interfacing.h"

void deactivateCannonMode();
char *readInput();
char *other_tab(char *input,int *count);
void activateCannonMode();

#endif