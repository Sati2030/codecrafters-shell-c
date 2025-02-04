#include "redirection.h"
#include "argumenter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Redirects stdout or stderr to a file if specified
void redirection(Arguments *args){

    //If there is a > then it directs the stdout or stderr to the desired file and removes from args list
    for(int i = (args->count)-1; i >= 0; i--){
        if(!strcmp(args->arguments[i],"2>")){  //If write stderr to a file
            freopen(args->arguments[i+1],"w",stderr);
            args->count -= 2;
            free(args->arguments[i+1]);
            free(args->arguments[i]);
            args->arguments = realloc(args->arguments,args->count * sizeof(char *));
        }
        else if(!strcmp(args->arguments[i],">") || !strcmp(args->arguments[i],"1>")){  //If write stdout to a file
            freopen(args->arguments[i+1],"w",stdout);
            args->count -= 2;
            free(args->arguments[i+1]);
            free(args->arguments[i]);
            args->arguments = realloc(args->arguments,args->count * sizeof(char *));
        }
        else if(!strcmp(args->arguments[i],">>") || !strcmp(args->arguments[i],"1>>")){ //If append stdout to a file
            freopen(args->arguments[i+1],"a",stdout);
            args->count-=2;
            free(args->arguments[i+1]);
            free(args->arguments[i]);
            args->arguments = realloc(args->arguments,args->count*sizeof(char*));
        }
        else if(!strcmp(args->arguments[i],"2>>")){ //If append stderr to a file
            freopen(args->arguments[i+1],"a",stderr);
            args->count-=2;
            free(args->arguments[i+1]);
            free(args->arguments[i]);
            args->arguments = realloc(args->arguments,args->count*sizeof(char*));
        }
    }

}