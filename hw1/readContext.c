//
// Created by arulselvanmadhavan on 1/16/16.
//

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <sys/ucontext.h>

int main(int argc, char *argv[]){
    ucontext_t mycontext;
    FILE * headerReader;
    headerReader=fopen("header.txt","r");
    if(headerReader == NULL){
        printf("Error occured in opening the file");
        exit(EXIT_FAILURE);
    }
    read(fileno(headerReader), &mycontext, sizeof(mycontext));
    printf("%ld",mycontext.uc_flags);
//    setcontext(&mycontext);

}