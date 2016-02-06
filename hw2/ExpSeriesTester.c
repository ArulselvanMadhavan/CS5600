//
// Created by arulselvanmadhavan on 1/30/16.
//

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>


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


double  calculateExponentialSeries(double x,double n)
{
    double total = 0.0;
    double i = 1.0;
    while(i <= n)
    {
        total += (pow(x, i) / factorial((int)i));
        i++;
    }
    return total+1.0;
}


int main(int argc,char * argv[])
{
    int i=1;//Skip the binary name
    int x=0;
    int n=0;
    char * xflag ="-x";
    char * nflag ="-n";

    double result = 0.1;
    while(argv[i] != NULL){
        if(strcmp(argv[i],xflag) == 0){
            i++;
            x = atof(argv[i]);
        }else if(strcmp(argv[i],nflag) == 0){
            i++;
            n = atof(argv[i]);
        }
        i++;
    }
    result = calculateExponentialSeries(x,n);
    printf("Result:\t%lf\n",result);
    FILE * fw = fopen("test.txt","w+");
    fprintf(fw,"%lf",result);
    fclose(fw);
}