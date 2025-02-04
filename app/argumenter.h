#ifndef _ARGUMENTER_H
#define _ARGUMENTER_H

    typedef struct{
        char **arguments;
        int count;
    } Arguments;

    Arguments arg_arrayer(char *input);

#endif