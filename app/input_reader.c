#include "input_reader.h"
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

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

  char c;
  int i = 0;

  while(read(STDIN_FILENO,&c,1) > 0){

    //Handles input of ENTER
    if(c == '\n'){
      printf("\n");
      break;
    }

    //Handles input of BACKSPACE
    if(c == 127){
      backspace(input,&i);
      continue;
    }
    
    //Handles autocompletion
    if(c == 9){
      if(!strcmp(input,"ech")){ //Of Echo
        complete_input(input,"echo ",&i);
      }
      else if(!strcmp(input,"exi")){ //Of exit
        complete_input(input,"exit ",&i);
      }
      else if(!strcmp(input,"typ")){ //Of type
        complete_input(input,"type ",&i);
      }
      else{
        other_tab(input,&i);
      }
      continue;
    }

    //If none of the special characters is pressed:
    printf("%c",c);

    input[i++] = c;
    input[i] = '\0';

  }

  return;
}

//Handles the tab functionality if its not an in-built command
void other_tab(char *input,int *count){
  
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
        qsort(entries.arguments,entries.count,sizeof(char*),comparatorFunction);
        for(int i = 0; i < entries.count; i++){
          printf("%s  ",entries.arguments[i]);
        }
        printf("\n");
        printf("$ ");
        printf("%s",input);
      }
      else if(c == 127){ //If backspace is pressed
        backspace(input,count);
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
void backspace(char *input, int *count){
  
  if(*count > 0){
    printf("\b \b");
    input[--(*count)] = '\0';
  }

  return;
}

int comparatorFunction(const void *a, const void *b){
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