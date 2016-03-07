//
// Created by arulselvanmadhavan on 1/29/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

typedef enum { false, true } bool;

double factorial(double n)
{
    double fact = 1;
    while(n > 0)
    {
//        printf("soFar:%lf\tcurrent:%lf\n",fact,n);
        fact = fact * n;
        n--;
    }
    return fact;
}
double calculateExponentialSeries(double x,double n){
    double num = pow(x,n);
    double denom = factorial(n);
//    printf("Size:%ld\t%ld\n",sizeof(num),sizeof(denom));
//    printf("num: %lf\tdenom: %lf\tres: %lf\n",num,denom,num/denom);
    return num/denom;
}

int main(int argc, char* argv[]){
    int i=1;//Skip the binary name
    int x=0;
    int n=0;
    char * xflag ="-x";
    char * nflag ="-n";
    char * pipeflag ="-pipe";
    char * filename = malloc(50);
    char * errorFilename = malloc(50);

    bool isPipe = false;
    double result = 0.1;
    while(argv[i] != NULL){
        if(strcmp(argv[i],xflag) == 0){
            i++;
            x = atoi(argv[i]);
        }else if(strcmp(argv[i],nflag) == 0){
            i++;
            n = atoi(argv[i]);
        }else if(strcmp(argv[i],pipeflag) == 0){
            isPipe = true;
        }
        i++;
    }
    result = calculateExponentialSeries(x,n);
    if(isPipe == false) {
        printf("x^n/n!:%.32lf", result);
    }else{
        struct timespec tim,tim2;
        tim.tv_sec = log2(n);
//        nanosleep(&tim,&tim2);

//        sprintf(filename,"log%d.txt",n);
//        FILE * logFile = fopen(filename,"w+");

        int retval = write(STDOUT_FILENO,&result,sizeof(result));

//        if(retval != 8){
//            sprintf(errorFilename,"error%d.txt",n);
//            FILE * errorLog = fopen(errorFilename,"w+");
//            fprintf(errorLog,"Not able to write the result");
//            fclose(errorLog);
//        }
//        fprintf(logFile,"%.32lf",result);

//        fprintf(stderr,"%d <%.32lf>\n",n,result);

//        fclose(logFile);
    }
}