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
#include <assert.h>
#include <sys/ucontext.h>

/*
 * Global Variables
 */
ucontext_t currentContext;

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


char* stradd(const char* a, const char* b){
    size_t len = strlen(a) + strlen(b);
    char *ret = (char*)malloc(len * sizeof(char) + 1);
    *ret = '\0';
    return strcat(strcat(ret, a) ,b);
}

writeToIndexFile(int fileno, char * words[]){
    char *str = stradd(words[0], "\n");
    write(fileno,str, sizeof(str));
}

char * getStringFromLong(long ulong_value){
    const int n = snprintf(NULL, 0, "%lu", ulong_value);
    assert(n > 0);
    char buf[n+1];
    char * start = &buf[0];
    int c = snprintf(start, n+1, "%lu", ulong_value);
    assert(buf[n] == '\0');
    assert(c == n);
    return start;
}

void saveContext(){
    FILE * headerFile;
    getcontext(&currentContext);
//    if (result != 0) {
//        printf("Something went wrong while getting usercontext\n");
//    }
    headerFile = fopen("header.txt", "w+");
    write(fileno(headerFile), &currentContext, sizeof(currentContext));
    printf("%ld\n",currentContext.uc_flags);
    printf("Stack Size:%d\n",currentContext.uc_stack.ss_flags);
    printf("rip:%ld",
           currentContext.uc_mcontext.fpregs->rip);
    fclose(headerFile);
}

void createImage(int signo) {
    if (signo == SIGUSR2) {
        printf("Received a request to create Image file");
        saveContext();

    }
    FILE *fp;
    FILE *fw;
    FILE *headerFile;
    FILE *indexFileWriter;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int addressesCount = 2, newAddr = 0;
    char *words[addressesCount];
    int counter = 0;
    int i = 0;
    char *startAddr = NULL;
    char *endaddr = NULL;
    char *res;
    char *longToString;
    long start = 0;
    long end = 0;
    int number = 0;
    long numberOfBytesWritten = 0;
    long totalNumberOfBytes = 0;
    int result = -1;
    ucontext_t currentContext;
    char *permissions[addressesCount + 1];
    char *permOfProc;
    fp = fopen("/proc/self/maps", "r");
    fw = fopen("test.txt", "w+");
    indexFileWriter = fopen("index.txt","w+");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    if (fw == NULL) {
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        getwords(line, words);
        start = strtol(words[0], NULL, 16);
        end = strtol(words[1], NULL, 16);

        numberOfBytesWritten = (long) write(fileno(fw), (void *) start,
                                            (size_t) end - start);
        if (numberOfBytesWritten == -1) {
            printf("Invalid - %ld\n", numberOfBytesWritten);
        } else {
            counter++;
            totalNumberOfBytes += numberOfBytesWritten;
            printf("%ld\n", totalNumberOfBytes);
            fprintf(indexFileWriter,"%s\t%ld\t%s\n",words[0],end - start,words[2]);
        }
    }
    fclose(fp);
    fclose(fw);
    fclose(indexFileWriter);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}

/*
 * Constructor
 */

__attribute__ ((constructor))
void myconstructor() {
    signal(SIGUSR2, createImage);
}

/**
 * Main Program
 */
int main(int argc, char *argv[]) {
    int counter = 0;
    char *words[3];
    while(1){
        sleep(1);
        counter++;
        printf("%d\t",counter);
        fflush(stdout);
    }
    createImage(0);
//    getWordsWithPermissions(line,words,3);
}

