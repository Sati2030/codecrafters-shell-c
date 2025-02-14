#include "prog_interfacing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>


//Executes any program that is listed in the PATH variable
void program_execution(Arguments *args){

    char *prog = args->arguments[0];
  
    //Forks a new process to execute program in 
    pid_t pr = fork();

    switch (pr)
    {
        //If parent process
        default:
            wait(NULL);
        break;
        //If child process
        case 0:
            if(execvp(prog,args->arguments) == -1){
                perror("Error executing program \n");
                _exit(1);
            }
            fflush(stdout);
        break;
        //If fork fails
        case -1:
            printf("Fork failed\n");
        break;
    }

    return;
}

//Gets the path variable
char *get_path(){
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

    return path;
}

//Checks if a command is valid and returns type
char *path_checker(char *input){

    char *path = get_path();

    //Tokenizes the Path
    char *dir = strtok(path,":");

    //Checking loop
    while(dir != NULL){
        
        int len = strlen(dir) + strlen(input) + 2;
        //Allocates memory for search
        char *search = (char*)malloc(len);
        if(search == NULL){
            perror("Memory allocation failed (search in valid_command function)\n");
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

