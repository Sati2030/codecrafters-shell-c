#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "argumenter.h"
#include "prog_interfacing.h"

typedef Arguments SearchResults;

void deactivateCannonMode();
void activateCannonMode();
int getRow();
int comparator_function(const void *a,const void *b);
void readInput(char *input);
void backspace(char *input,int *cursor,int *count);
void moveInputRight(char *input,int n,int *cursor,int *count);
void moveCursor(int *cursor,int count, int pos);
void other_tab(char *input,int *cursor,int *count);
void get_matches(SearchResults *entries,char *input);
void file_autocompletion(char *input, int *cursor, int *count, int fileNo);
void get_dir_entries(SearchResults *files,char *input);
void complete_input(char *input, char *completion, int *cursor, int *count, int argNo);
char *get_token(char *input, int argNo);


#endif