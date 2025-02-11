#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "input_reader.h"
#include "argumenter.h"
#include "commands.h"
#include "redirection.h"
#include "prog_interfacing.h"

int main() {

  while(1){
    //Get the original stdout and stderr file descriptor
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
    // Flush after every printf
    setbuf(stdout, NULL);
    //For the first input
    printf("$ ");
    
    //Deactivates cannonical mode of the terminal
    deactivateCannonMode();

    //Reads the input by the user
    char input[1024] = {0};
    readInput(input);

    //If input is empty continue
    if(!strcmp(input,"")){
      continue;
    }

    //Creates a dynamic array of arguments
    Arguments args = {NULL,0};
    arg_arrayer(&args,input);
    int typeFlag = valid_command(args.arguments[0]);
  

    //Checks if valid command is passed to input
    if(typeFlag < 0){
      printf("%s: command not found\n",args.arguments[0]);
      continue;
    }

    //Handling of the redirection commands
    redirection(&args);

    //Handles the commands if shell builtins
    if(!typeFlag){
      command_handling(&args);
    }
    else{
      //Adds a NULL to the end of the argument array
      args.arguments = realloc(args.arguments,(args.count + 1)*sizeof(char*));
      if(!args.arguments){
        perror("Error reallocating memory for NULL terminator of arguments array\n");
        exit(1);
      }
      args.arguments[args.count] = NULL;

      program_execution(&args);
    }

    //Bring the stdout back to the console
    fflush(stdout);
    fflush(stderr);
    dup2(stdout_fd, fileno(stdout));
    dup2(stderr_fd, fileno(stderr));
    close(stdout_fd);
    close(stderr_fd);
    

    //Frees the dynamically allocated array of arguments
    for(int i = 0; i < args.count; i++){
      free(args.arguments[i]);
    }
    free(args.arguments);

    args.count = 0;
  
  }

  return 0;

}


