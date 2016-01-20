//
// Created by arulselvanmadhavan on 1/18/16.
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
#include <string.h>

ucontext_t oldContext;

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

mapDataToMemory(long start,long size,char * perm,FILE * ckptReader){
    void *startaddr = (void *)start;
    void * pmap;
    int prot = 0;
    int mem=0;
    //Order matters
    prot = getProtValue(perm);
    perm+=3;
    mem = (*perm=='p')?MAP_PRIVATE:MAP_SHARED;
    pmap = mmap(startaddr,size,prot,mem,fileno(ckptReader),0);
    if(pmap != MAP_FAILED){
        fseek(ckptReader, size, SEEK_CUR);
    }
}

rFromFile(){

    FILE * ckptReader;
    FILE * logFile;
    FILE *contextFile;
    ckptReader = fopen("myckpt","r+");
    logFile = fopen("readLog.txt","w+");
    contextFile = fopen("header","r+");
    char c;
    int offset = 0;
    int headerLen =0;
    int readSoFar = 0;
    int readStatus = 0;
    int numOfWords = 0;
    long start=0;
    long size = 0;
    char *perm;
    char * words[3];
    int cur_pos = 0;
    long fpos = 0;
    long spos = 0x5300000;
    long epos = 0x5320000;
    void * p = (void *)spos;
    void * e = (void *)epos;
    void *pmap;


    //Move stack pointer to new position
    pmap = mmap(p,epos-spos,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
    if(pmap == MAP_FAILED){
        exit(EXIT_FAILURE);
    }
    asm volatile ("mov %0,%%rsp;" : : "g" (e) : "memory");

    //unmap current stack
    unmapCurrentStack();

    //Get user context
    offset = read(fileno(contextFile),&oldContext,sizeof(oldContext));
    printf("%d\n",offset);
    fclose(contextFile);



    while(feof(ckptReader) != 1 && ferror(ckptReader) !=1) {

        //Step1:First read the integer
        readStatus = fread(&headerLen, sizeof(int), 1, ckptReader);
        if (readStatus != 1) {
            printf("%d\n",feof(ckptReader));
            continue;
//            exit(EXIT_FAILURE);
        }
        fprintf(logFile,"Pos After reading header Length:\t%ld\n", ftell(ckptReader));
        readSoFar += sizeof(int);
        char *str = malloc(headerLen);

        //Step2: Read the actual String
        readStatus = fread(str, headerLen, 1, ckptReader);
        fprintf(logFile,"Header: %s", str);
        fprintf(logFile,"Pos after reading Header:\t%ld\n", ftell(ckptReader));
        if (readStatus != 1) {
            exit(EXIT_FAILURE);
        }
        readSoFar += headerLen;

        //Step3:Parse the Header Info
        numOfWords = parseIndexFile(str, words);
        if (numOfWords != 0) {
            start = strtol(words[0], NULL, 16);
            size = strtol(words[1], NULL, 10);
            perm = words[2];
            fpos = ftell(ckptReader);
            mapDataToMemory(start,size,perm,ckptReader);
            readSoFar += size;
        }
        fpos = ftell(ckptReader);
        fprintf(logFile,"Pos after reading memory:\t%ld\n", ftell(ckptReader));
        fprintf(logFile,"\n");
    }
//    swapcontext(&curContext,&oldContext);
    printf("Done");
    fclose(ckptReader);
    setcontext(&oldContext);
}

/*
 * Main
 */
int main(int argc,char * argv){
    //To-Do get file name from argv
    rFromFile();
}