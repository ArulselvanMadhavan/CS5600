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
//    else if(strcmp(mechanism,"poll")==0)
//    {
//        pollmechanism(counter,pipefds);
//    } else if(strcmp(mechanism,"epoll")== 0)
//    {
//        epollmechanism(counter,pipefds);
//    }
//    else if(strcmp(mechanism,"sequential")==0)
//    {
//        sequentialMechanism(counter,pipefds,pidArray);
//    }
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
    while (counter > 0) {
        fd_set dup = master;
        retval = select(max_fd + 1, &dup, NULL, NULL, NULL);
        if (retval < 0) {
            printf("Select returned an incorrect value of %d\n", retval);
            perror("select()");
        }
        else if (retval) {
            /* check which fd is avaialbe for read */
            for (i = 0; i <= counter; i++) {
                int fd = pipefds[i * 2];
                printf("Reading FD:%d\n",fd);
                if (FD_ISSET(fd, &dup)) {
                    printf("%d is set\n", fd);
                    nbytes = read(fd, &d, sizeof(double));
                    printf("Bytes:%d\n", nbytes);
                    printf("Value:%lf\n", d);
                    total = total + d;
                    counter--;
                }
            }

//            counter--;
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
    struct pollfd fds[1];
    int timeout;
    int pret;
    fds[0].fd = pipefds[0];
    fds[0].events = 0;
    fds[0].events |= POLL_IN;
    timeout = 3000;

    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;

    while (counter > 0) {
        pret = poll(fds, 1, timeout);
        if (pret < 0) {
            perror("poll()");
        }
        else if (pret) {
            nbytes = read(pipefds[0], &d, sizeof(double));
            printf("Bytes:%d\n", nbytes);
            printf("Value:%lf\n", d);
            total = total + d;
            counter--;
        }
        else {
            printf("No data to read\n");
        }
    }
    printf("Total:\t%lf", total + 1);
}


epollmechanism(int counter, int pipefds[]) {
    int epfd = epoll_create(1);
    struct epoll_event ev, events[1];
    int nfds;
    if (epfd < 0) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = pipefds[0];
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipefds[0], &ev) == -1) {
        perror("epll_ctl");
        exit(EXIT_FAILURE);
    }

    int nbytes = 0;
    double d = 0.1;
    double total = 0.0;
    while (counter > 0) {
        nfds = epoll_wait(epfd, events, 1, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        else if (nfds > 0) {
            while (nfds > 0) {
                if (events[nfds - 1].data.fd == pipefds[0]) {
                    if (events[nfds - 1].events & EPOLLHUP || events[nfds - 1].events & EPOLLERR) {
                        close(pipefds[0]);
                        exit(EXIT_FAILURE);
                    }
                    nbytes = read(pipefds[0], &d, sizeof(double));
                    printf("Bytes:%d\n", nbytes);
                    printf("Value:%lf\n", d);
                    total = total + d;
                    counter--;
                }
                nfds--;
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

    while (counter > 0) {
        curr_pid = waitpid(pidArray[counter - 1], &status, options);
        if (curr_pid == pidArray[counter - 1]) {
            nbytes = read(pipefds[0], &d, sizeof(double));
            printf("Bytes:%d\n", nbytes);
            printf("Value:%lf\n", d);
            total = total + d;
            printf("PID:%d\n", (int) pidArray[counter - 1]);
            counter--;
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