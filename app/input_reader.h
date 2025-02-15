#ifndef _MY_INPUT_READER_H
#define _MY_INPUT_READER_H
#include "argumenter.h"
#include "prog_interfacing.h"

typedef Arguments SearchResults;

void deactivateCannonMode();
void readInput(char *input);
void other_tab(char *input,int *cursor,int *count);
void file_autocompletion(char *input, int *cursor, int *count, int fileNo);
void complete_input(char *input, char *completion, int *cursor, int *count, int argNo);
void backspace(char *input,int *cursor,int *count);
void cursor_handling(int *cursor,int *count, int action);
void moveInputRight(char *input,int n,int *cursor,int *count);
int getRow();
void moveCursor(int *cursor,int count, int pos);
char *get_token(char *input, int argNo);
void get_matches(SearchResults *entries,char *input);
void get_dir_entries(SearchResults *files,char *input);
int comparator_function(const void *a,const void *b);
void activateCannonMode();

#endif