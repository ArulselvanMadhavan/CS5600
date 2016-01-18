//
// Created by arulselvanmadhavan on 1/16/16.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ucontext.h>
#include <ucontext.h>

/*
 * Global Variables
 */
char * indexFile="index.txt";
char * dataFile ="test.txt";
char * headerFile = "header.txt";

parseIndexFile(char * str,char * words[]){
    int numOfWords = 0;
    while(isspace(*str)){
        str++;
    }
    while (1) {
        if(*str == '\0'){
            return numOfWords;
        }
        words[numOfWords++] = str;
        while (*str != '\t' && *str !='\n') {
            str++;
        }
        *str++ = '\0';
    }
}

void test(){
    sleep(1);
    printf("something");

}

splitLine(char *line,char * words[]){
    char *p = line;
    int numOfWords = 0;
    //Remove space at start
    while(isspace(*p)){
        p++;
    }
    words[numOfWords++]=p;
    while(*p != '-'){
        p++;
    }
    *p++='\0';

    //
    words[numOfWords++] = p;
    while(*p != ' '){
        p++;
    }
    *p++='\0';
}

unmapCurrentStack(){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int i=0;
    char * stackText = "[stack]";
    char * words[2];
    fp = fopen("/proc/self/maps", "r");
    long start =0;
    long end = 0;
    int res;
    while ((read = getline(&line, &len, fp)) != -1){
        if(strstr(line,stackText) != NULL){
            printf("Found!!!\n");
            splitLine(line,words);
            start = strtol(words[0],NULL,0);
            end = strtol(words[1],NULL,0);
            res = munmap((void*) start,end - start);
            if(res == 0)
                printf("Done");
        }
    }
}

int getProtValue(char * perm){
    int val = PROT_READ;
    perm++;
    if(*perm=='w'){
        val |= PROT_WRITE;
    }
    *perm++;
    if(*perm =='x'){
        val |= PROT_EXEC;
    }
    *perm++;
    return val;
}

int mapDataToMemory(long start,long size,char * perm,int offset){
    void *startaddr = (void *)start;
    void * pmap;
    FILE *dataReader;
    int prot = 0;
    int mem=0;
    perm+=3;
    mem = (*perm=='p')?MAP_PRIVATE:MAP_SHARED;
    prot = getProtValue(perm);
//    printf("Prot Value:%d\t%c\t%d",prot,*perm,mem);
    dataReader = fopen(dataFile,"r+");
    pmap = mmap(startaddr,size,prot,mem,fileno(dataReader),offset);
    fclose(dataReader);
    return offset+size;
}

ucontext_t mycontext;
int main(int argc, char *argv[]){
    FILE *indexReader;
    FILE *contextReader;
    char *line = NULL;
    size_t len =0;
    ssize_t reader = -1;
    char *words[3];
    int numOfWords = 0;
    long start =0;
    long end = 0;
    long size = 0;
    long spos = 0x5300000;
    long epos = 0x5310000;
    void * p = (void *)spos;
    void * e = (void *)epos;
    void *pmap;
    char * perm;
    int offset = 0;

    //Move stack pointer to new position
    pmap = mmap(p,epos-spos,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
    if(pmap == MAP_FAILED){
        exit(EXIT_FAILURE);
    }
    asm volatile ("mov %0,%%rsp;" : : "g" (e) : "memory");
    unmapCurrentStack();

    //Read Usercontext
    contextReader = fopen(headerFile,"r+");
    read(fileno(contextReader),&mycontext, sizeof(mycontext));


    //Read the index file
    indexReader = fopen(indexFile,"r");
    while((reader = getline(&line,&len,indexReader)) != -1){
        numOfWords = parseIndexFile(line,words);
        if(numOfWords != 0){
            start = strtol(words[0],NULL,16);
            size = strtol(words[1],NULL,10);
            perm = words[2];
            offset = mapDataToMemory(start,size,perm,offset);
        }
    }
    printf("%ld",mycontext.uc_mcontext.fpregs->rip);
    setcontext(&mycontext);
    fclose(contextReader);
}