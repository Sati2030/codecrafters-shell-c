#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "argumenter.h"
#include "prog_interfacing.h"

typedef Arguments SearchResults;

void deactivateCannonMode();
void readInput(char *input);
void other_tab(char *input,int *count);
void complete_input(char *input, char *completion, int *count);
void backspace(char *input,int *count);
void get_matches(SearchResults *entries,char *input);
void activateCannonMode();

#endif