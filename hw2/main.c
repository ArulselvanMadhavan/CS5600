//
// Created by arulselvanmadhavan on 1/29/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <assert.h>

typedef enum {
    false, true
} bool;

int main(int argc, char *argv[]) {
    //Do the processing of input arguments here
    int i = 1;//Skip the binary name
    int x = 0;
    int n = 0;
    char *xflag = "-x";
    char *nflag = "-n";
    char *workerpathFlag = "--workerpath";
    char *mechanismFlag = "--wait_mechanism";
    char *workerpath = malloc(50);
    char *mechanism = malloc(50);

    double result;
    while (i < argc) {
        if (strcmp(argv[i], xflag) == 0) {
            i++;
            x = atoi(argv[i]);
        } else if (strcmp(argv[i], nflag) == 0) {
            i++;
            n = atoi(argv[i]);
        } else if (strcmp(argv[i], workerpathFlag) == 0) {
            i++;
            workerpath = argv[i];
            printf("workerpath:%s\n", workerpath);
        } else if (strcmp(argv[i], mechanismFlag) == 0) {
            i++;
            mechanism = argv[i];
            printf("Mechanism:%s\n", mechanism);
        }
        else {
            printf("%s is an Invalid flag\n", argv[i]);
            exit(EXIT_SUCCESS);
        }
        i++;//Move to the next flag
    }
    spawnWorkers(workerpath, mechanism, x, n);
}

spawnWorkers(char *workerpath, char *mechanism, int x, int n) {
//    Calculate the number of bits required to print x
//    and store it in a variable
    int length = snprintf(NULL, 0, "%d", x);
    char xAsStr[length];
    sprintf(xAsStr, "%d", x);

//    Create a pipe for each worker
    int counter = n;
    int pipefds[counter * 2];
    createPipes(pipefds, counter);

    pid_t pidArray[n];

    while (n > 0) {
        length = snprintf(NULL, 0, "%d", n);
        char nAsStr[length];
        sprintf(nAsStr, "%d", n);

        pidArray[n - 1] = fork();
        if (pidArray[n - 1] < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pidArray[n - 1] == 0) {
            char *args[] = {
                    workerpath,
                    "-x",
                    xAsStr,
                    "-n",
                    nAsStr,
                    "-pipe",
                    NULL
            };
            dup2(pipefds[(n - 1) * 2 + 1], STDOUT_FILENO);
            close(pipefds[(n - 1) * 2]);
            execv(args[0], args);
        }
        n--;
    }
//        close(pipefds[1]);

//        wait(NULL);
    if (strcmp(mechanism, "select") == 0) {
        selectMechanism(counter, pipefds);
    }
    else if(strcmp(mechanism,"poll")==0)
    {
        pollmechanism(counter,pipefds);
    }
    else if(strcmp(mechanism,"epoll")== 0)
    {
        epollmechanism(counter,pipefds);
    }
    else if(strcmp(mechanism,"sequential")==0)
    {
        sequentialMechanism(counter,pipefds,pidArray);
    }
    return 0;
}


selectMechanism(int counter, int pipefds[]) {

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    fd_set master;

    int max_fd = 0;
    FD_ZERO(&master);

    int i = 0;
    for (i = 0; i < counter; i++) {
        safe_fd_set(pipefds[i * 2], &master, &max_fd);
    }
    char buffer[8];
    double *result = malloc(sizeof(double));
    int retval;
    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;
    int counter_dup = counter;
    while (counter > 0) {
        fd_set dup = master;
        retval = select(max_fd+1, &dup, NULL, NULL, NULL);
        if (retval < 0) {
            printf("Select returned an incorrect value of %d\n", retval);
            perror("select()");
        }
        else if (retval) {
            /* check which fd is avaialbe for read */
            for (i = 0; i < counter_dup; i++) {
                int fd = pipefds[i * 2];
                if (FD_ISSET(fd, &dup)) {
                    printf("Reading data for %d from fd:%d\n",i+1 ,fd);
                    nbytes = read(fd, &d, sizeof(double));
                    printf("Bytes:%d\n", nbytes);
                    printf("Value:%lf\n", d);
                    total = total + d;
                    counter--;
                }
            }
        }
        else {
            printf("No data to read\n");
        }
        printf("Remaining processes:\t%d\n", counter);
    }
    close(pipefds[0]);
    printf("Result:\t%lf", total + 1);
}


pollmechanism(int counter, int pipefds[]) {
    struct pollfd pollfds[counter];
    int i =0;
    for(i=0;i<counter;i++)
    {
        pollfds[i].fd = pipefds[i*2];
        pollfds[i].events = 0;
        pollfds[i].events |= POLL_IN;
    }
//    struct pollfd fds[1];
    int timeout;
    int pret;
    timeout = 3000;

    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;
    int counter_dup = counter;
    while (counter > 0) {
        pret = poll(pollfds, counter_dup, timeout);
        if (pret < 0) {
            perror("poll()");
        }
        else if (pret) {
            for(i=0;i<counter_dup;i++)
            {
                if(pollfds[i].revents && POLL_IN)
                {
                    printf("Worker id %d from fd:%d\n",i+1,pollfds[i].fd);
                    nbytes = read(pollfds[i].fd, &d, sizeof(double));
                    printf("Bytes:%d\n", nbytes);
                    printf("Value:%lf\n", d);
                    total = total + d;
                    counter--;
                }
            }
        }
        else {
            printf("No data to read\n");
        }
    }
    printf("Total:\t%lf", total + 1);
}


epollmechanism(int counter, int pipefds[]) {
    int epfd = epoll_create(counter);
    struct epoll_event events[counter];
    int nfds;
    if (epfd < 0)
    {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
    int i;
    for(i=0;i<counter;i++)
    {
        events[i].events = EPOLLIN;
        events[i].data.fd = pipefds[i*2];
    }

    for(i =0;i<counter;i++)
    {
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipefds[i*2], &events[i]) == -1)
        {
            perror("epll_ctl");
            exit(EXIT_FAILURE);
        }
    }

    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;
    int counter_dup = counter;
    while (counter > 0)
    {
        nfds = epoll_wait(epfd, events, counter_dup, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        else if (nfds > 0)
        {
            printf("Read %d events from the pipes\n",nfds);
            for(i =0;i<nfds;i++)
            {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    close(events[i].data.fd);
                    exit(EXIT_FAILURE);
                }
                printf("Reading from FD:%d\n",events[i].data.fd);
                nbytes = read(events[i].data.fd, &d, sizeof(double));
                printf("Bytes:%d\n", nbytes);
                printf("Value:%lf\n", d);
                total = total + d;
                counter--;
            }
        }
    }
    printf("Total:\t%lf", total + 1);
}


sequentialMechanism(int counter, int pipefds[], pid_t pidArray[]) {
    int status;
    int options = WUNTRACED | WCONTINUED;
    pid_t curr_pid;

    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;
    int i =0;
    for(i=0;i<counter;)
    {
        curr_pid = waitpid(pidArray[counter - i - 1], &status,0);
        printf("PID:%d\n",curr_pid);
        printf("Status:%d\n",status);
        if(WIFEXITED(status))
        {
            nbytes = read(pipefds[i*2], &d, sizeof(double));
            printf("Bytes:%d\n", nbytes);
            printf("Value:%lf\n", d);
            total = total + d;
            d = 0.0;
            i++;
        }
    }
    printf("Total:\t%lf\n", total + 1);
}


createPipes(int pipefds[], int pipes) {
    int i;
    printf("Pipes:%d\n", pipes);
    for (i = 0; i < pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("Couldn't Pipe");
            exit(EXIT_FAILURE);
        }
        printf("Read Pipe Id:%d\n", *(pipefds + i * 2));
    }
}


/* add a fd to fd_set, and update max_fd */
int safe_fd_set(int fd, fd_set *fds, int *max_fd) {
    assert(max_fd != NULL);
    printf("Setting the %d fd\n", fd);
    FD_SET(fd, fds);
    if (fd > *max_fd) {
        *max_fd = fd;
    }
    printf("Max fd:%d\n", *max_fd);
    return 0;
}

/* clear fd from fds, update max fd if needed */
int safe_fd_clr(int fd, fd_set *fds, int *max_fd) {
    assert(max_fd != NULL);

    FD_CLR(fd, fds);
    if (fd == *max_fd) {
        (*max_fd)--;
    }
    return 0;
}