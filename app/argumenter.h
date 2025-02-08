#ifndef _ARGUMENTER_H
#define _ARGUMENTER_H

    typedef struct{
        char **arguments;
        int count;
    } Arguments;

    void arg_arrayer(Arguments *args,char *input);

#endif