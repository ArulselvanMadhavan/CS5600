//
// Created by arulselvanmadhavan on 1/17/16.
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
#include <string.h>

struct header{
    char * headLine;
};

typedef struct header Object;
Object* Object_new(char *hl) {
    Object* p = malloc(sizeof(Object));
    p->headLine = hl;
    return p;
}

char * getNextInValidPosition(char * p,char separator){
    while (!isspace(*p) && *p != '\0' && *p != separator)
        p++;
    return p;
}

/**
 * Input 1: A pointer to a string line.
 * Input 2: A pointer to array of pointers to characters.
 */
getwords(char *line, char *words[]) {
    char *p = line;
    int nwords = 0;
    while (isspace(*p))
        p++;

    words[nwords++] = p;
    p = getNextInValidPosition(p,'-');
    *p++ = '\0';

    //Get End address
    words[nwords++] = p;
    p = getNextInValidPosition(p,' ');
    *p++='\0';

    //Get Permissions
    words[nwords++] = p;
    p = getNextInValidPosition(p,' ');
    *p++='\0';
}

wToFile(){
    FILE * writer;
    FILE *mapsFile;
    struct header h1;
    struct header h2;
    int count =0;
    char *line = NULL;
    size_t len =0;
    ssize_t reader;
    long start=0;
    long end = 0;
    char *words[2];
    long nob=0;
    struct header *info;
    int counter = 0;
    long totalNumberOfBytes=0;
    mapsFile = fopen("/proc/self/maps", "r");
    writer = fopen("master","w+");
    char *headerInfo;
    int headerLen=0;
    while ((reader = getline(&line, &len, mapsFile)) != -1){
        getwords(line, words);
        start = strtol(words[0], NULL, 16);
        end = strtol(words[1], NULL, 16);



        int headerLen = snprintf(NULL, 0, "%s\t%ld\t%s\n",words[0],end - start,words[2]);
        headerLen++;
        char * buffer = malloc(headerLen);
        snprintf(buffer,headerLen,"%s\t%ld\t%s\n",words[0],end - start,words[2]);
        //First write the string length
        nob = write(fileno(writer),&headerLen,sizeof(int));
        //Second write the string
        nob = write(fileno(writer),buffer,headerLen);
//        printf("String Bytes:%ld\n",totalNumberOfBytes);
        nob = (long) write(fileno(writer), (void *) start,
                                            (size_t) end - start);
        totalNumberOfBytes+=nob;
//        printf("Data Bytes:%ld\n",totalNumberOfBytes);
    }
}

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

rToFile(){
    FILE * reader;
    int count = 0;
    reader = fopen("master","r+");
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

    while(!feof(reader)) {
        //Step1:First read the integer
        readStatus = fread(&headerLen, sizeof(int), 1, reader);
        if (readStatus != 1) {
            exit(EXIT_FAILURE);
        }
        printf("Current Pos After reading integer%ld\n", ftell(reader));
        readSoFar += sizeof(int);
        char *str = malloc(headerLen);
        //Step2: Read the actual String
        readStatus = fread(str, headerLen, 1, reader);
        printf("Val: %s", str);
        printf("Current Pos after reading string%ld\n", ftell(reader));
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
        }
        readSoFar += size;

        fseek(reader, size, SEEK_CUR);
        printf("Current Pos after reading seeking %ld\n", ftell(reader));
    }
    fclose(reader);
}

int main(int argc, char * argv[]){
    long ret = 0;
    wToFile();
    rToFile();

    ret = strtol("400000",NULL,16);
    printf("\n%ld\t%lx\t",ret,ret);
    ret = strtol("402000",NULL,16);
    printf("\n%ld\t%lx\t",ret,ret);
}