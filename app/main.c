#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

const char *builtin[5] = {"exit","type","echo","pwd","cd"};
int static count = 0;

void echo(char **arg,int num_args);
void type(char **arg, int num_args);
void exit_(char **arg, int num_args);
void cd(char **arg);
void pwd();
char **arg_arrayer(char *input);
char* valid_command(char *input);
void command_handling(char **arguments,int num_args);
void program_execution(char **arg, char *prog);


int main() {

  while(1){
    //Get the original stdout and stderr file descriptor
    int stdout_fd = dup(fileno(stdout));
    int stderr_fd = dup(fileno(stderr));
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
    char **args = arg_arrayer(input);

    //Checks if valid command is passed to input
    char *temp = valid_command(args[0]);

    //If there is a > then it directs the stdout or stderr to the desired file and removes from args list
    for(int i = count-1; i >= 0; i--){
      if(!strcmp(args[i],"2>")){
        freopen(args[i+1],"w",stderr);
        count -= 2;
        free(args[i+1]);
        free(args[i]);
        args = realloc(args,count * sizeof(char *));
      }
      else if(!strcmp(args[i],">") || !strcmp(args[i],"1>")){
        freopen(args[i+1],"w",stdout);
        count -= 2;
        free(args[i+1]);
        free(args[i]);
        args = realloc(args,count * sizeof(char *));
      }
    }

    //Handles the commands if shell builtins
    if(temp){
      if(!strcmp(temp,"a shell builtin")){
        free(temp);
        command_handling(args,count);
      }
      else if(args[0]){ //Else executes the program listed in the PATH variable
        free(temp);

        //Adds NULL terminator to the argument array
        args = realloc(args,(count+1)*sizeof(char*));
        if(args == NULL){
          printf("Error reallocating memory for arguments array (NULL terminator)\n");
          exit(1);
        }
        args[count] = NULL;

        program_execution(args,args[0]);
      }
    }
    else{
      //Prints (input command): command not found
      printf("%s: command not found\n",input);
    }

    //Bring the stdout back to the console
    fflush(stdout);
    fflush(stderr);
    dup2(stdout_fd, fileno(stdout));
    dup2(stderr_fd, fileno(stderr));
    close(stdout_fd);
    close(stderr_fd);

    //Frees the dynamically allocated array of arguments
    for(int i = 0; i < count; i++){
      free(args[i]);
    }
    free(args);

    count = 0;
  
  }

  return 0;
}

char **arg_arrayer(char *input){
  char **arguments = NULL;  //Arguments array
  char buffer[1024];  //Temporary buffer to keep track of the argument placed
  int i,j = 0;  //i = for the loop counter, j = for the position in the buffer
  int sq_flag = 0; //Flag that activates if a single quote is opened
  int dq_flag = 0; //Flag that activates if a double quote is opened
  int bs_flag = 0; //Flag that activates if a backslash is opened

  for(i = 0; i <= strlen(input); i++){

    //Handling of single quotes
    if(input[i] == '\''){
      if(!dq_flag && !bs_flag){ //If not already in a double quote or backslash
        sq_flag = !sq_flag;
        continue; //Skips the quote in the buffer
      }
    }

    //Handling of double quotes
    if(input[i] == '"'){
      if(!sq_flag && !bs_flag){ //If not already in single quotes or backslash
        dq_flag = !dq_flag;
        continue; //Skips the quote in the buffer 
      } 
    }

    //Handling of backslashes
    if(input[i] == '\\'){
      if(dq_flag){  //If inside double quotes special meaning is reatained before another, " and $
        if((input[i+1] == '\\' || input[i+1] == '"' || input[i+1] == '$') && input[i-1] != '\\'){
          bs_flag = 1;
          continue; //Skips the backslash in the buffer
        }
      }
      else if(!sq_flag){ //Else if not inside single quotes
        bs_flag = 1;
        continue; //Skipts the backslash in the buffer
      }
    }

    //Handling if there is multiple spaces in between arguments (but not inside quotation marks)
    if(!sq_flag && !dq_flag && input[i] == ' ' && input[i-1] == ' '){
      continue;
    }

    /*If not inside quotes, and not after a backslash, the space or '\0'
    will store the buffer into an argument space*/
    if(!sq_flag && !dq_flag && ((input[i] == ' ' && !bs_flag) || input[i] == '\0')){

      buffer[j] = '\0';

      arguments = realloc(arguments,(count +1)* sizeof(char *));
      if(arguments == NULL){
        printf("Memory allocation failed (arguments)\n");
        exit(1);
      }

      arguments[count] = malloc((strlen(buffer) + 1) * sizeof(char));
      if(arguments[count] == NULL){
        printf("Memory allocation failed argument %d\n",count);
        exit(1);
      }

      strcpy(arguments[count],buffer);

      count++;
      j = 0;
      continue;
    }

    //Copy the input char in to the buffer
    buffer[j++] = input[i];

    //Makes sure backslash is deactivated after taking in the next character
    bs_flag = 0;
  }

  return arguments;
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
  if(!strcmp(arguments[0],"echo")){
    echo(arguments,num_args);
  }
  else if(!strcmp(arguments[0],"type")){
    type(arguments,num_args);
  }
  else if(!strcmp(arguments[0],"exit")){
    exit_(arguments,num_args);
    }
  else if(!strcmp(arguments[0],"pwd")){
    pwd();
  }
  else if(!strcmp(arguments[0],"cd")){
    cd(arguments);
  }

  return;
}

//Executes any program that is listed in the PATH variable
void program_execution(char **arg, char *prog){
  
  //Forks a new process to execute program in 
  pid_t pr = fork();

  switch (pr)
  {
    //If child process
    case 0:
      fflush(stdout);
      if(execvp(prog,arg) == -1){
        printf("Error executing program \n");
        _exit(1);
      }
      break;
    //If fork fails
    case -1:
      printf("Fork failed\n");
      break;
    //If parent process
    default:
      wait(NULL);
      break;
  }

  return;
}

void cd(char **arg){
   
  //If there is no argument or the argument is "~" go to the HOME directory
  if(!arg[1] || !strcmp(arg[1],"~")){
    if(!chdir(getenv("HOME"))){
      return;
    }
    else{
      printf("Error changing to home directory\n");
      return;
    }
  }
  //Else move to the specified directory
  else if(!chdir(arg[1])){
    return;
  }
  //If specified directory does not exist
  else{
    printf("cd: %s: No such file or directory\n",arg[1]);
  }

  return;
}

//Function for the pwd command
void pwd(){
  char *cwd = getcwd(NULL,0);

  if(cwd){
    printf("%s\n",cwd);
    free(cwd);
  }
  else{
    printf("Error locating current working directory\n");
  }

  return;
}

//Function for the echo command
void echo(char **arg,int num_args){

  for(int i = 1; i<(num_args); i++){
    printf("%s",arg[i]);
    if(i == (num_args-1)){
      printf("\n");
    }
    else{
      printf(" ");
    }
  }

  return;

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

  return;
}

//Function for the exit command
void exit_(char **arg, int num_args){

  //Handles if there is just exit
  if(num_args < 2){
    printf("Not enough arguments\n");
    return;
  }

  if(!strcmp(arg[1],"0")){
    for(int i = 0; i<num_args;i++){
      free(arg[i]);
    }
    free(arg);
    exit(0);
  }
  else{
    printf("Not a valid argument\n");
    return;
  }

  return;
}
