#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "argumenter.h"
#include "prog_interfacing.h"

typedef Arguments SearchResults;

void deactivateCannonMode();
void readInput(char *input);
void other_tab(char *input,int *cursor,int *count);
void complete_input(char *input, char *completion, int *count);
void backspace(char *input,int *cursor,int *count);
void cursor_handling(int *cursor,int *count, int action);
void moveInputRight(char *input,int *cursor,int *count);
int getRow();
void get_matches(SearchResults *entries,char *input);
int comparator_function(const void *a,const void *b);
void activateCannonMode();

#endif