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
#include <string.h>

ucontext_t currentContext;
int writeCounter = 0;

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

wToHeaderFile(){
    FILE * headerFile;
    headerFile = fopen("header","w+");
    write(fileno(headerFile),&currentContext,sizeof(currentContext));
    if(fclose(headerFile) !=0){
        printf("Error occured while closing the file");
    }
}

wToFile(){

    FILE * writer;
    FILE *mapsFile;
    FILE *logFile;
    char *line = NULL;
    size_t len =0;
    ssize_t reader;
    long start=0;
    long end = 0;
    char *words[2];
    int nob=0;
    long total=0;
    int res = 0;
    mapsFile = fopen("/proc/self/maps", "r");
    writer = fopen("myckpt","w+");
    logFile = fopen("writeLog.txt","w+");
    while ((reader = getline(&line, &len, mapsFile)) != -1){
        /*
         * Parse a line in maps file
         */
        fprintf(logFile,"Reading Line: %s",line);
        getwords(line, words);
        start = strtol(words[0], NULL, 16);
        end = strtol(words[1], NULL, 16);

        if(*words[2] == '-' || (end-start)<=0){
            //Skip non-readable sections
            printf("Skipping non-readable section");
            continue;
        }
        /*
         * First calculate the length of the header Text
         */
        int headerLen = snprintf(NULL, 0,
                                 "%s\t%ld\t%s\n",words[0],end - start,words[2]);
        headerLen++;
        nob = write(fileno(writer),&headerLen,sizeof(int));
        total+=nob;
        fprintf(logFile,"HeaderLength:%d\tBytes:%d\n",headerLen,nob);

        /*
         * Create Header text
         */
        char * buffer = malloc(headerLen);
        snprintf(buffer,headerLen,"%s\t%ld\t%s\n",words[0],end - start,words[2]);
        nob = write(fileno(writer),buffer,headerLen);
        total+=nob;
        fprintf(logFile,"HeaderText:%s\tBytes:%d\n",buffer,nob);

        /*
         * Write the memory object
         */
        nob = (long) write(fileno(writer), (void *) start,
                           (size_t) end - start);
        total+=nob;
        fprintf(logFile,"Memory Object Size:%ld\tBytes:%d\n",end-start,nob);
    }

    fprintf(logFile,"Total:%ld\n",total);
    /*
    * Close the Resources and free the memory
    */
    fclose(mapsFile);
    fclose(logFile);

    /**
     *
     */

    writeCounter = 1;
    getcontext(&currentContext);
    printf("\nSet context done\n");
    if(writeCounter == 1) {
        printf("\nSleeping\n");
//        sleep(5);
        nob = write(fileno(writer),&currentContext,
                    sizeof(currentContext));
        printf("Context Bytes written:%d\n",nob);
        printf("Total+CurrentContext:%ld\n",
                total+sizeof(currentContext));
        fclose(writer);
        exit(EXIT_SUCCESS);
    }
}
/*
 * Create Checkpoint image
 */
void createImage(int signo) {
    if (signo == SIGUSR2) {
            printf("\n!!!!Received a request to create Image file!!!\n");
            wToFile();
        //Contents are dumped. Now I can manipulate Global Variables
//            writeCounter = 1;
//            getcontext(&currentContext);
//        printf("\nSet context done\n");
//        if(writeCounter == 1) {
//            printf("\nSleeping\n");
//            sleep(5);
//            wToHeaderFile();
//            exit(EXIT_SUCCESS);
//        }
    }
}


__attribute__ ((constructor))
void init_signal() {
    signal(SIGUSR2, createImage);
}