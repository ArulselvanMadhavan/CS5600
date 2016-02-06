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
char * ckptFile;

parseIndexFile(char *str, char *words[]) {
    int numOfWords = 0;
    while (isspace(*str)) {
        str++;
    }
    while (1) {
        if (*str == '\0') {
            return numOfWords;
        }
        words[numOfWords++] = str;
        while (*str != '\t' && *str != '\n') {
            str++;
        }
        *str++ = '\0';
    }
}

splitLine(char *line, char *words[]) {
    char *p = line;
    int numOfWords = 0;
    //Remove space at start
    while (isspace(*p)) {
        p++;
    }
    words[numOfWords++] = p;
    while (*p != '-') {
        p++;
    }
    *p++ = '\0';

    //
    words[numOfWords++] = p;
    while (*p != ' ') {
        p++;
    }
    *p++ = '\0';
}

unmapCurrentStack() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int i = 0;
    char *stackText = "[stack]";
    char *words[2];
    fp = fopen("/proc/self/maps", "r");
    long start = 0;
    long end = 0;
    int res;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (strstr(line, stackText) != NULL) {
            printf("Found!!!\n");
            splitLine(line, words);
            start = strtol(words[0], NULL, 0);
            end = strtol(words[1], NULL, 0);
            res = munmap((void *) start, end - start);
            if (res == 0)
                printf("Done");
        }
    }
}

int getProtValue(char *perm) {
    int val = PROT_READ;
    perm++;
    if (*perm == 'w') {
        val |= PROT_WRITE;
    }
    *perm++;
    if (*perm == 'x') {
        val |= PROT_EXEC;
    }
    *perm++;
    return val;
}


mapDataToMemory(long start, long size, char *perm,FILE* ckptReader) {
    void *startaddr = (void *) start;
    void *pmap;
    int prot = 0;
    int mem = 0;
    long fpos = 0;

    //Order matters
    prot = PROT_READ|PROT_WRITE|PROT_EXEC;
    perm += 3;
    mem = (*perm == 'p') ? MAP_PRIVATE : MAP_SHARED;
    mem |= MAP_FIXED;
    mem |= MAP_ANONYMOUS;
    pmap = mmap(startaddr, size, prot, mem,-1, 0);
    if(pmap == MAP_FAILED){
        printf("Error in mapping file at: %lx",start);
    }
//    fseek(ckptReader,size,SEEK_CUR);
    return size;
}

rFromFile() {
    FILE *ckptReader;
    FILE *logFile;
    FILE *contextFile;
    ckptReader = fopen(ckptFile, "r+");
    logFile = fopen("readLog.txt", "w+");
//    contextFile = fopen("header", "r+");
    char c;
    int offset = 0;
    int headerLen = 0;
    int readSoFar = 0;
    int readStatus = 0;
    int numOfWords = 0;
    long start = 0;
    long size = 0;
    char *perm;
    char *words[3];
    int cur_pos = 0;
    long numOfCharsPrinted = 0;
    long spos = 0x5300000;
    long epos = 0x5320000;
    long fpos = 0;
    void *p = (void *) spos;
    void *e = (void *) epos;
    void *pmap;
    long eof =0;
    long startOfContext =0;
    //Move stack pointer to new position
    pmap = mmap(p, epos - spos, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (pmap == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }
    asm volatile ("mov %0,%%rsp;" : : "g" (e) : "memory");

    //unmap current stack
    unmapCurrentStack();

    //Seek end to find the length of the file
    fseek(ckptReader, 0L, SEEK_END);
    eof = ftell(ckptReader);
    fseek(ckptReader, 0L, SEEK_SET);

    //Get user context
    //Seek the end of file - sizeof(ucontext_t)
    startOfContext = eof - sizeof(ucontext_t);
    fseek(ckptReader,startOfContext,SEEK_CUR);
    fpos = ftell(ckptReader);
    printf("Size of the context: %ld\n",sizeof(oldContext));
    offset = read(fileno(ckptReader), &oldContext, sizeof(oldContext));
    printf("Context Size: %d\n", offset);

    fseek(ckptReader, 0L, SEEK_SET);

    while(ftell(ckptReader) < startOfContext){
        printf("Current File Pos:%ld",ftell(ckptReader));
//    while (feof(ckptReader) != 1 && ferror(ckptReader) != 1) {

        //Step1:First read the integer
        readStatus = fread(&headerLen, sizeof(int), 1, ckptReader);
        if (readStatus != 1) {
            printf("%d\n", feof(ckptReader));
            continue;
        }
        fprintf(logFile,
                "Pos After reading header Length:\t%ld\n",
                ftell(ckptReader));

        readSoFar += sizeof(int);
        char *str = malloc(headerLen);

        //Step2: Read the actual String
        readStatus = fread(str, headerLen, 1, ckptReader);
        fprintf(logFile, "Header: %s", str);
        fprintf(logFile, "Pos after reading Header:\t%ld\n", ftell(ckptReader));
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
            int prot = getProtValue(perm);
            mapDataToMemory(start, size, perm,ckptReader);

            readStatus = fread((void *)start,size,1,ckptReader);

            int res = mprotect((void *)start,size,prot);
            if(res !=0){
                printf("Error in updating the protection:%d",errno);
            }
            if(readStatus != 1){
                printf("Error occured while"
                               " reading from file at:%ld"
                               "into memory: %lx",fpos,start);
            }
            fpos = ftell(ckptReader);
            readSoFar += size;
        }
        fprintf(logFile, "Pos after reading memory:\t%ld\n", ftell(ckptReader));
        fprintf(logFile, "\n");
    }
    fpos = fclose(logFile);
    if (fpos != 0) {
        printf("Error occured while closing logFile: %d\n", errno);
    }
    fclose(ckptReader);
    setcontext(&oldContext);
    printf("Restored");
}

/*
 *
 */
int main(int argc, char **argv) {
    //To-Do get file name from argv
    printf("%s\n",argv[1]);
    ckptFile = argv[1];
    rFromFile(ckptFile);
}