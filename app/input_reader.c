#include "input_reader.h"
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define FORWARD 1
#define BACKWARD 0
#define END 2
#define BEGINNING 3

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

  //Handles escape characters
   if(nBytes == 3){
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
    //Everything else
    else if(nBytes == 1){

      //CTRL+(A,B,E,A)
      if(c[0] == '\x06'){
        cursor_handling(&cursorPos,&i,FORWARD);
        continue;
      }
      else if(c[0] == '\x02'){
        cursor_handling(&cursorPos,&i,BACKWARD);
        continue;
      }
      else if(c[0] == '\x05'){
        cursor_handling(&cursorPos,&i,END);
        continue;
      }
      else if(c[0] == '\x01'){
        cursor_handling(&cursorPos,&i,BEGINNING);
        continue;
      }

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
          complete_input(input,"echo ",&cursorPos,&i);
        }
        else if(!strcmp(input,"exi")){ //Of exit
          complete_input(input,"exit ",&cursorPos,&i);
        }
        else if(!strcmp(input,"typ")){ //Of type
          complete_input(input,"type ",&cursorPos,&i);
        }
        else{ //Command autocompletion
          other_tab(input,&cursorPos,&i);
        }
        
        continue;
      }

      //If the user is typing before the end of the input
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
  
  int row = getRow();

  //Forward
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
  //Backwards
  else if(action == 0){
    if(*cursor == 0){
      return;
    }
    else{
      printf("\x1B[D");
      (*cursor)--;
      return;
    }
  }
  //End of line
  else if(action == 2){
    if(*cursor == *count){
      return;
    }
    else{
      *cursor = *count;
      printf("\x1B[%d;%dH",row,(*cursor+3));
      return;
    }
  }
  //Beginning of line
  else if(action == 3){
    *cursor = 0;
    printf("\x1B[%d;%dH",row,3);
    return;
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
    complete_input(input,entries.arguments[0],cursor,count);
    printf(" ");
    input[(*count)++] = ' ';
    input[*count] = '\0';
    (*cursor)++;
  }
  //If there are multiple results
  else if(entries.count > 1){ 

    //Checks if there is an autcompletion that prefixes all other options
    for(int p = 0; p < entries.count ; p++){

      char *comparator = entries.arguments[p]; //Used as the autocompletion option to compare if prefix
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

      //If the current comparator prefixes all other autcomplete options then chose
      if(validFlag){
        complete_input(input,comparator,cursor,count);
        goto exit;
      }
    }

    printf("\a");
    if(read(STDIN_FILENO,&c,1) > 0){
      if(c == 9){ //If tab is pressed again
        int row = getRow();
        printf("\x1B[%d;%dH",row,(*count+3)); //Ensures cursor is at end of line before new line
        printf("\n");
        qsort(entries.arguments,entries.count,sizeof(char*),comparator_function);
        for(int i = 0; i < entries.count; i++){
          printf("%s  ",entries.arguments[i]);
        }
        printf("\n");
        printf("$ ");
        printf("%s",input);
        row = getRow();
        printf("\x1B[%d;%dH",row,(*cursor+3));
      }
      else if(c == 127){ //If backspace is pressed
        backspace(input,cursor,count);
      }
      else{ //If another key is pressed
        if(*cursor < *count){
          moveInputRight(input,cursor,count);
        }
        printf("%c",c);
        input[(*cursor)++] = c;
        input[++(*count)] = '\0';
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

//Handles the autcompetion of files
/*void file_autocompletion(char *input, int *cursor, int*count){

  SearchResults files = {NULL,0};
  if(input[*count-1] == ' '){
    get_dir_entries(&files,NULL);
  }
  else{
    char *tok = strtok(input," ");
    char *last;
    while(tok){
      last = tok;
      tok = strtok(NULL," ");
    }
    get_dir_entries(&files,last);
  }

  //If there is only one match
  if(files.count == 1){
    complete_input(input,files.arguments[0],cursor,count);
  }
  else{
    printf("\n");

  }


  for(int i = 0; i < files.count; i++){
    free(files.arguments[i]);
  }
  free(files.arguments);


}*/

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

//Function to add entries of a directory
/*void get_dir_entries(SearchResults *files,char *input){
  
  struct dirent *entry;
  DIR *directory = opendir(".");

  if(directory == NULL){
    perror("Error opening cwd\n");
    exit(1);
  }

  while((entry = readdir(directory))){
    if(input){
      if(strncmp(entry->d_name,input,sizeof(input))){
        addToArray(files,entry->d_name);
      }
    }
    else{
      addToArray(files,entry->d_name);
    }
  }
}
*/


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

            //Adds entry to array
            addToArray(entries,entry->d_name);

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
void complete_input(char *input,char *completion,int *cursor, int *count){

  //Ensures cursor is at end of line
  int row = getRow();
  printf("\x1B[%d;%dH",row,(*count+3));
 
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

  //Puts the cursor counter in the end of line
  *cursor = *count;

  return;

}

//Handling of backspacing a character
void backspace(char *input,int *cursor, int *count){
  
  if(*cursor > 0){
    printf("\b \b");
    if(*cursor < *count){
      for(int i= --(*cursor); i < *count; i++){
        input[i] = input[i+1];
        printf("%c",input[i]);
      }
      (*count)--;
      printf(" ");
      int row = getRow();
      printf("\x1B[%d;%dH",row,(*cursor+3));
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