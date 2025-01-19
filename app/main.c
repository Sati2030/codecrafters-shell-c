#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char *builtin[3] = {"exit","type","echo"};

void echo(char **arg,int num_args);
void type(char **arg, int num_args);
void exit_(char **arg, int num_args);
char* valid_command(char *input);
void command_handling(char **arguments,int num_args);


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

    //Creates a dynamic array of arguments
    char *argum = strtok(input," ");
    char **arguments = NULL;
    int count = 0;
    int length;
  
    while(argum){
      arguments = realloc(arguments,(count+1) * sizeof(char *));
      if(arguments == NULL){
        printf("Memory allocation failed (arguments)\n");
        exit(1);
      }
      
      length = strlen(argum);

      arguments[count] = malloc(length * sizeof(char));
      if(arguments[count] == NULL){
        printf("Memory allocation failed (argument)\n");
        exit(1);
      }

      strcpy(arguments[count],argum);

      count++;

      argum = strtok(NULL, " ");
    }  

    char *temp = valid_command(arguments[0]);

    //Handles the commands
    if(temp){
      free(temp);
      command_handling(arguments,count);
    }
    else{
      //Prints (input command): command not found
      printf("%s: command not found\n",input);
    }

    //Frees the dynamically allocated array of arguments
    for(int i = 0; i < count; i++){
      free(arguments[i]);
    }
    free(arguments);
  
  }
  return 0;
}



//Checks if a command is valid and returns type
char* valid_command(char *input){

  //First check if the command is not a shell builtin
  int builtinnum = sizeof(builtin)/sizeof(builtin[0]);
  for(int i = 0; i<builtinnum;i++){
    if(strcmp(builtin[i],input) == 0){
      char *shell_builtin = (char *)malloc(16);
      if(shell_builtin == NULL){
        printf("Memory allocation failed (shell builtin type)\n");
        exit(1);
      }
      strcpy(shell_builtin,"a shell builtin");
      return shell_builtin;
    }
  }

  //Retrieves the PATH 
  const char *path_env = getenv("PATH");
  if(path_env == NULL){
    perror("Erorr retrieving PATH\n");
    exit(1);
  }

  //Duplicates the PATH string
  char *path = strdup(path_env);
  if(path == NULL){
    perror("There was an error duplicating the PATH\n");
    exit(1);
  }

  //Tokenizes the Path
  char *dir = strtok(path,":");

  //Checking loop
  while(dir != NULL){
    
    int len = strlen(dir) + strlen(input) + 2;
    //Allocates memory for search
    char *search = (char*)malloc(len);
    if(search == NULL){
      printf("Memory allocation failed (search in valid_command function)\n");
      exit(1);
    }

    //Adds the path to search and the command
    snprintf(search,len,"%s/%s",dir,input);

    //If command is found in PATH directory return directory
    if(!access(search,F_OK)){
      free(path);
      return(search);
    }
    else{
      free(search);
      dir = strtok(NULL,":");
    }
  }

  free(path);
  return NULL;
}

//Directs to its respecitve fucntion the command that is passed into the function
void command_handling(char **arguments,int num_args){
  if(strcmp(arguments[0],"echo") == 0){
    echo(arguments,num_args);
  }
  else if(strcmp(arguments[0],"type") == 0){
    type(arguments,num_args);
  }
  else if(strcmp(arguments[0],"exit") == 0){
      exit_(arguments,num_args);
    }
}


//Function for the echo command
void echo(char **arg,int num_args){

  for(int i = 1; i<num_args; i++){
    printf("%s",arg[i]);
    if(i == (num_args-1)){
      printf("\n");
    }
    else{
      printf(" ");
    }
  }

}

//Function for the type command
void type(char **arg, int num_args){

  char *type;

  for(int i = 1;i<num_args;i++){
    type = valid_command(arg[i]);
    if(type){
      printf("%s is %s\n",arg[i],type);
      free(type);
    }
    else{
      printf("%s: not found\n",arg[i]);
    }
  }
}

//Function for the exit command
void exit_(char **arg, int num_args){

  if(!strcmp(arg[1],"0")){
    for(int i = 0; i<num_args;i++){
      free(arg[i]);
    }
    free(arg);
    exit(0);
  }

}
