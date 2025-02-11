#include "input_reader.h"
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define FORWARD 1
#define BACKWARD 0

struct termios ogTerminal;

//Deactivates cannonical mode for instant input reading
void deactivateCannonMode(){
  struct termios terminal;

  if(tcgetattr(STDIN_FILENO, &ogTerminal) == -1){
    perror("Error getting terminal attributes\n");
    exit(1);
  }

  terminal = ogTerminal;

  terminal.c_lflag &= ~(ICANON|ECHO|ISIG);

  if(tcsetattr(STDIN_FILENO,TCSANOW,&terminal) == -1){
    perror("Error setting terminal attributes\n");
    exit(1);
  }

  return;

}

//Reads the input from the terminal
void readInput(char *input){

  char c[3];
  int i = 0;
  int cursorPos = 0;
  int nBytes;

  while((nBytes = read(STDIN_FILENO,c,sizeof(c))) > 0){

   if(nBytes == 3){

      //Handles input of arrows
      if(c[0] == '\x1B' && c[1] == '['){

        switch(c[2]){
          case 'A': break;
          case 'B': break;
          case 'C': 
            cursor_handling(&cursorPos,&i,FORWARD);
            break;
          case 'D':
            cursor_handling(&cursorPos,&i,BACKWARD);
        }
        tcflush(STDIN_FILENO, TCIFLUSH);
        memset(c,0,sizeof(c));
        continue;

      }
      else{
        continue;
      }
    }
    else if(nBytes == 1 && c[0] == '\x1B'){
      continue;
    }
    else if(nBytes == 1){

      //Handles input of ENTER
      if(c[0] == '\n'){
        printf("\n");
        break;
      }

      //Handles input of BACKSPACE
      if(c[0] == 127){              
        backspace(input,&cursorPos,&i);
        continue;
      }
      
      //Handles autocompletion
      if(c[0] == 9){
        if(!strcmp(input,"ech")){ //Of Echo
          complete_input(input,"echo ",&i);
          cursorPos = i;
        }
        else if(!strcmp(input,"exi")){ //Of exit
          complete_input(input,"exit ",&i);
          cursorPos = i;
        }
        else if(!strcmp(input,"typ")){ //Of type
          complete_input(input,"type ",&i);
          cursorPos = i;
        }
        else{
          int oldi = i;
          other_tab(input,&cursorPos,&i);
          if(oldi < i){
            cursorPos = i;
          }
        }
        continue;
      }


      if(cursorPos < i){
        moveInputRight(input,&cursorPos,&i);
      }

      //If none of the special characters is pressed:
      printf("%c",c[0]);
      
      input[cursorPos++] = c[0];
      input[++i] = '\0';

    }

  }

  return;
}

//Handling of the cursor
void cursor_handling(int *cursor,int *count, int action){
  
  if(action == 1){
    if(*cursor == *count){
      return;
    }
    else{
      printf("\x1B[C");
      (*cursor)++;
      return;
    }
  }
  else{
    if(*cursor == 0){
      return;
    }
    else{
      printf("\x1B[D");
      (*cursor)--;
      return;
    }
  }

}

//Moves input to the right if the cursor is behind the end of the
void moveInputRight(char *input,int *cursor,int *count){

  int row = getRow();

  for(int i = *count ; i > *cursor; i--){
    input[i] = input[i-1];
  }
  input[*cursor] = ' ';
  
  printf("\x1B[3G");
  printf("%s",input);
  printf("\x1B[%d;%dH",row,(*cursor+3));
  
}

//Handles the tab functionality if its not an in-built command
void other_tab(char *input,int *cursor,int *count){
  
  //Creates a dyanmic array of matches
  SearchResults entries = {NULL,0};
  get_matches(&entries,input);

  char c; //Used for reading next character if there is multiple results

  //If there is only one command that can be autocompleted
  if(entries.count == 1){
    complete_input(input,entries.arguments[0],count);
    printf(" ");
    input[(*count)++] = ' ';
    input[*count] = '\0';
  }
  else if(entries.count > 1){ //If there are multiple results

    for(int p = 0; p < entries.count ; p++){

      char *comparator = entries.arguments[p];
      int validFlag;

      for(int w = 0; w < entries.count; w++){
        if(w == p){ //Skips same entry
          continue;
        }
        if(strncmp(comparator,entries.arguments[w],strlen(comparator))){ //If the comparator does not prefix another command
          validFlag = 0;
          break;
        }
        validFlag = 1;
      }

      if(validFlag){
        complete_input(input,comparator,count);
        goto exit;
      }
    }

    printf("\a");
    if(read(STDIN_FILENO,&c,1) > 0){
      if(c == 9){ //If tab is pressed again
        printf("\n");
        qsort(entries.arguments,entries.count,sizeof(char*),comparator_function);
        for(int i = 0; i < entries.count; i++){
          printf("%s  ",entries.arguments[i]);
        }
        printf("\n");
        printf("$ ");
        printf("%s",input);
      }
      else if(c == 127){ //If backspace is pressed
        backspace(input,cursor,count);
      }
      else{ //If another key is pressed
        printf("%c",c);
        input[(*count)++] = c;
        input[*count] = '\0';
      }
    }
  }
  else{
    printf("\a");
  }

  exit:

  //Frees the entries
    for(int k = 0; k<entries.count;k++){
    free(entries.arguments[k]);
  }
  free(entries.arguments);

  return;

}

//Gets the row the cursor is at
int getRow(){
  char buf[16];
  int j = 0;
  int row = -1;

  tcflush(STDIN_FILENO, TCIFLUSH);
  write(STDOUT_FILENO,"\x1B[6n",4);
  while(j < sizeof(buf) -1){
    if(read(STDIN_FILENO,&buf[j],1) != 1){
      break;
    }
    if(buf[j]=='R'){
      break;
    }
    j++;
  }
  buf[j] = '\0';
  sscanf(buf,"\x1B[%d",&row);
  tcflush(STDIN_FILENO,TCIFLUSH);
  return row;
}

//Function that handles getting functions for autocompletion
void get_matches(SearchResults *entries, char *input){

  //Initialize entries array and retrieve PATH
  char *path = get_path();

  //Tokenizes the PATH
  char *dir = strtok(path,":");

  //Checks every directory for matches of the prefix
  while(dir){
    //Open directory for check
    DIR *directory = opendir(dir);
    if(directory){
      //Check every item inside directory
      struct dirent *entry;
      while((entry=readdir(directory))){

        //If entry starts with the same as the input
        if(!strncmp(entry->d_name,input,strlen(input))){

          char search[1024];
          snprintf(search,sizeof(search),"%s/%s",dir,entry->d_name);

          //If the search is an executable add to the entires array
          if(!access(search,F_OK)){

            //Handles duplicates
            for(int i = 0; i<entries->count; i++){
              if(!strcmp(entry->d_name,entries->arguments[i])){
                goto ex;
              }
            }

            entries->arguments = realloc(entries->arguments,(entries->count+1)*sizeof(char*));
            if(!entries->arguments){
              perror("Erorr allocating memory for entries array\n");
              exit(1);
            }
            entries->arguments[entries->count] = strdup(entry->d_name);
            if(!entries->arguments[entries->count]){
              perror("Error allocating memory for entry in entries array\n");
              exit(1);
            }
            entries->count++;
          }
          
        }

        ex:;

      }

      closedir(directory);
    }
    dir = strtok(NULL,":");
  }
  free(path);
  return;

}

//Function that handles autocompletion
void complete_input(char *input,char *completion,int *count){
 
  //Moves the pointer of the completion to after the input
  int ogInpLen = strlen(input);
  completion += ogInpLen;
  //Starts filling the screen and input array
  int newCompLen = strlen(completion);
  printf("%s",completion);
  for(int i = 0; i<newCompLen; i++){
    input[(*count)++] = completion[i];
  }
  //Null terminates and returns completion pointer to original char
  input[*count] = '\0';
  completion -= ogInpLen;

  return;

}

//Handling of backspacing a character
void backspace(char *input,int *cursor, int *count){
  
  if(*cursor > 0){
    printf("\b \b");
    if(*cursor < *count){
      input[--(*cursor)] = ' ';
      for(int i= *cursor; i < (*count); i++){
        input[i] = input[i+1];
      }
      int row = getRow();
      printf("\x1B[%d;%dH",row,(*count+3));
      printf("\b \b");
      (*count)--;
      printf("\x1B[3G");
      printf("%s",input);
      printf("\x1B[%d;%dH",row,*cursor+3);
    }
    else{
      input[--(*count)] = '\0';
      (*cursor)--;
    }

  }

  return;
}

//Comparator function for sorting
int comparator_function(const void *a, const void *b){
  return strcmp(*(const char**)a,*(const char**)b);
}

//Restores original terminal settings
void activateCannonMode(){
    if(tcsetattr(STDIN_FILENO,TCSANOW,&ogTerminal) == -1){
        perror("Error resetting terminal attributes\n");
        exit(1);
    }
    return;
}