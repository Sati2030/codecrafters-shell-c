#include "prog_interfacing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

const char *builtin[5] = {"echo","type","exit","pwd","cd"};

//Executes any program that is listed in the PATH variable
void program_execution(Arguments *args){

    char *prog = args->arguments[0];
  
    //Forks a new process to execute program in 
    pid_t pr = fork();

    switch (pr)
    {
        //If child process
        case 0:
        fflush(stdout);
        if(execvp(prog,args->arguments) == -1){
            perror("Error executing program \n");
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
char *valid_command(char *input){

    //First check if the command is not a shell builtin
    int builtinnum = sizeof(builtin)/sizeof(builtin[0]);
    for(int i = 0; i<builtinnum;i++){
        if(strcmp(builtin[i],input) == 0){
            return "a shell builtin";
        }
    }

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

//Gets result matches
Arguments get_matches(char *input){

    //Initialize entries array and retrieve PATH
    Arguments entries = {NULL,0};
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

                        entries.arguments = realloc(entries.arguments,(entries.count+1)*sizeof(char*));
                        if(!entries.arguments){
                            perror("Erorr allocating memory for entries array\n");
                            exit(1);
                        }
                        entries.arguments[entries.count] = strdup(entry->d_name);
                        if(!entries.arguments[entries.count]){
                            perror("Error allocating memory for entry in entries array\n");
                            exit(1);
                        }
                        entries.count++;
                    }
                }
            }
            closedir(directory);
        }
        dir = strtok(NULL,":");
    }
    free(path);
    return entries;


}