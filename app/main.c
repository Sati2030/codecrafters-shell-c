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
    // Uncomment this block to pass the first stage
    printf("$ ");
    
    //Deactivates cannonical mode of the terminal
    deactivateCannonMode();

    //reads input
    char *input = readInput();

    //Creates a dynamic array of arguments
    Arguments args = arg_arrayer(input);

    free(input);

    //Checks if valid command is passed to input
    char *temp = valid_command(args.arguments[0]);

    //Handling of the redirection commands
    redirection(&args);

    //Handles the commands if shell builtins
    if(temp){
      if(!strcmp(temp,"a shell builtin")){
        command_handling(&args);
      }
      else if(strcmp(args.arguments[0],"\0")){ //Else executes the program listed in the PATH variable
        free(temp);

        //Adds NULL terminator to the argument array
        args.arguments = realloc(args.arguments,(args.count+1)*sizeof(char*));
        if(args.arguments == NULL){
          printf("Error reallocating memory for arguments array (NULL terminator)\n");
          exit(1);
        }
        args.arguments[args.count] = NULL;

        program_execution(&args);
      }
    }
    else{
      //Prints (input command): command not found
      printf("%s: command not found\n",args.arguments[0]);
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


