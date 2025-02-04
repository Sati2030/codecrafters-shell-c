#include "argumenter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Organizes input into a dynamically allocated array
Arguments arg_arrayer(char *input){

    Arguments args = {NULL,0};
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

        args.arguments = realloc(args.arguments,(args.count +1)* sizeof(char *));
        if(args.arguments == NULL){
            perror("Memory allocation failed (arguments)\n");
            exit(1);
        }

        args.arguments[args.count] = malloc((strlen(buffer) + 1) * sizeof(char));
        if(args.arguments[args.count] == NULL){
            perror("Memory allocation failed argument\n");
            exit(1);
        }

        strcpy(args.arguments[args.count],buffer);

        args.count++;
        j = 0;
        continue;
        }

        //Copy the input char in to the buffer
        buffer[j++] = input[i];

        //Makes sure backslash is deactivated after taking in the next character
        bs_flag = 0;
    }

    return args;
}