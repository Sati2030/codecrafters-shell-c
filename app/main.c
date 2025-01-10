#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
  const char *name;
  const char *type;
} Command;

const Command commands[] = {
  {"echo","builtin"},
  {"exit","builtin"},
  {"type","builtin"},
  {NULL,NULL}
};

void echo(char *arg);
void type(char *arg);
void exit_(char *arg);
Command valid_command(char *input);
void command_handling(Command command,char *argument);


int main() {

  while(1){
    // Flush after every printf
    setbuf(stdout, NULL);
    // Uncomment this block to pass the first stage
    printf("$ ");
    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);
    //Removes the new line of the input buffer
    input[strlen(input) - 1] = '\0';

    //Returns a command if there is a valid command
    Command command = valid_command(input);

    //Handles the commands
    if(command.name != NULL){
      char *argument = input + strlen(command.name) + 1;
      command_handling(command,argument);
    }
    else{
    //Prints (input command): command not found
    printf("%s: command not found\n",input);
    }
  }
  
  return 0;
}

//Checks if a command is valid and returns said command
Command valid_command(char *input){
  for(int i = 0; commands[i].name != NULL; i++){

    size_t command_len = strlen(commands[i].name);
    
    if(strncmp(input, commands[i].name, command_len) == 0 ){

      char following_char = input[command_len];
      if(following_char == ' ' || following_char == '\0'){
        return commands[i];
      }

    }
  }
  return commands[sizeof(commands)/sizeof(commands[0]) -1];
}

//Directs to its respecitve fucntion the command that is passed into the function
void command_handling(Command command, char *argument){
  if(strcmp(command.name,"echo") == 0){
    echo(argument);
  }
  else if(strcmp(command.name,"type") == 0){
    type(argument);
  }
  else if(strcmp(command.name,"exit") == 0){
    exit_(argument);
  }
}

//Function for the echo command
void echo(char *arg){
  printf("%s\n",arg);
}

//Function for the type command
void type(char *arg){
  Command command = valid_command(arg);
  if(command.name != NULL){
    printf("%s is a shell %s\n",command.name,command.type);
  }
  else{
    printf("%s: not found\n",arg);
  }
}

//Function for the exit command
void exit_(char *arg){

  char *endptr;
  long int value = strtol(arg, &endptr, 10);

  if(*endptr != '\0' || value != 0){
    printf("invalid argument\n");
  }
  else{
    exit(0);
  }
}
