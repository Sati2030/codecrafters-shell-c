#include "input_reader.h"
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
char *readInput(){

  char *input = NULL;
  char c;
  int i = 0;

  while(read(STDIN_FILENO,&c,1) > 0){

    //Handles input of ENTER
    if(c == '\n'){
      input = realloc(input,(i+1)*sizeof(char));
      printf("\n");
      input[i] = '\0';
      break;
    }

    //Handles input of BACKSPACE
    if(c == 127){
      if(i > 0){
        input = realloc(input,(i)*sizeof(char));
        printf("\b \b");
        input[--i] = '\0';
      }
      continue;
    }
    
    //Handles autocompletion
    if(c == 9){
      if(!strcmp(input,"ech")){ //Of Echo
        printf("o ");
        input = realloc(input,(i+3)*sizeof(char));
        input[i++] = 'o';
        input[i++] = ' ';
        input[i] = '\0';
      }
      else if(!strcmp(input,"exi")){ //Of exit
        printf("t ");
        input = realloc(input,(i+3)*sizeof(char));
        input[i++] = 't';
        input[i++] = ' ';
        input[i] = '\0';
      }
      else if(!strcmp(input,"typ")){ //Of type
        printf("e ");
        input = realloc(input,(i+3)*sizeof(char));
        input[i++] = 'e';
        input[i++] = ' ';
        input[i] = '\0';
      }
      else{
        input = other_tab(input,&i);
        printf("\a"); //If no autocompletion available
      }
      continue;
    }

    //If none of the special characters is pressed:
    printf("%c",c);

    input = realloc(input,(i+2)*sizeof(char));

    input[i++] = c;
    input[i] = '\0';

  }

  return input;
}

char *other_tab(char *input,int *count){
  
  Entries entries = get_matches(input);

  if(entries.count == 1){
    int oginputlen = strlen(input);
    entries.entries[0] += strlen(input);
    printf("%s ",entries.entries[0]);
    input = realloc(input,(*count+strlen(entries.entries[0])+2)*sizeof(char));
    if(!input){
      perror("Error realocating memory for the input\n");
      exit(1);
    }
    int NEL = strlen(entries.entries[0]);
    for(int j = 0; j < NEL;j++){
      input[(*count)++] = entries.entries[0][j];
    }
    input[(*count)++] = ' ';
    input[*count] = '\0';
    entries.entries[0] -= oginputlen;
  }

  for(int k = 0; k<entries.count;k++){
    free(entries.entries[k]);
  }
  free(entries.entries);

  return input;

}

//Restores original terminal settings
void activateCannonMode(){
    if(tcsetattr(STDIN_FILENO,TCSANOW,&ogTerminal) == -1){
        perror("Error resetting terminal attributes\n");
        exit(1);
    }
    return;
}