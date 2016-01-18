//
// Created by arulselvanmadhavan on 1/16/16.
//

#include<stdio.h>
#include<signal.h>
#include<unistd.h>

void sig_handler(int signo)
{
    if (signo == SIGUSR2)
        printf("received SIGINT\n");
}

int main(void)
{
    if (signal(SIGUSR2, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    // A long long wait so that we can easily issue a signal to this process
    while(1)
        sleep(1);
    return 0;
}