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

//Handles the tab functionality if its not an in-built command
char *other_tab(char *input,int *count){
  
  //Creates an arguments dyanmic array to hold the results of the search
  Arguments entries = get_matches(input);
  char c; //Used for reading next character if there is multiple results

  //If there is only one command that can be autocompleted
  if(entries.count == 1){
    int oginputlen = strlen(input); 
    entries.arguments[0] += strlen(input); //Moves the result pointer to the char after the input
    printf("%s ",entries.arguments[0]);
    input = realloc(input,(*count+strlen(entries.arguments[0])+2)*sizeof(char)); //Reallocates the input array to hold the new input
    if(!input){
      perror("Error realocating memory for the input\n");
      exit(1);
    }
    //Starts adding the string to the input array
    int NEL = strlen(entries.arguments[0]);
    for(int j = 0; j < NEL;j++){
      input[(*count)++] = entries.arguments[0][j];
    }
    input[(*count)++] = ' ';
    input[*count] = '\0';
    entries.arguments[0] -= oginputlen; //Returns the original back to the original char that was pointed in the search result
  }
  else if(entries.count > 1){ //If there are multiple results
    printf("\a");
    if(read(STDIN_FILENO,&c,1) > 0){
      if(c == 9){ //If tab is pressed again
        printf("\n");
        for(int i = 0; i < entries.count; i++){
          printf("%s  ",entries.arguments[i]);
        }
        printf("\n");
        printf("$ ");
        printf("%s",input);
      }
      else{ //If another key is pressed
        printf("%c",c);
        input = realloc(input,(*count + 2)*sizeof(char));
        input[(*count)++] = c;
        input[*count] = '\0';
      }
    }
  }
  else{
    printf("\a");
  }


  for(int k = 0; k<entries.count;k++){
    free(entries.arguments[k]);
  }
  free(entries.arguments);

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