//
// Created by arulselvanmadhavan on 3/12/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

//static __thread char * name="";
//static __thread int i=0;
//void * myfunc(void * var)
//{
//    char * msg = (char *)var;
//    int i;
//    for(i =0;i<10;i++)
//    {
//        printf("%s\t%d\n",msg,i);
//        sleep(1);
//    }
//    return NULL;
//}
//
///**
// *
// */
//int main(int argc, char * argv[])
//{
//    pthread_t t1,t2;
//    char * msg1 = "T1";
//    char * msg2 = "T2";
//    int ret1,ret2;
//
//    ret1 = pthread_create(&t1,NULL,myfunc,(void *)msg1);
//    ret2 = pthread_create(&t2,NULL,myfunc,(void *)msg2);
//
//
//    pthread_join(t1,NULL);
//    pthread_join(t2,NULL);
//
//    printf("T1 returned:%d\n",ret1);
//    printf("T2 returned:%d\n",ret2);
//    return 0;
//}

static __thread int i = 0;
void* thread_func (void* data)
{
//    if(strcmp(name,"") == 0) {
//        name = (char *)data;
//        printf("%s Initialized\n",name);
//    }
    printf("Thread %ld received %d\n",(long)pthread_self(),*(int *)data);

    i = *(int *)data;
    int c = 0;
    for(c = 0;c<3;c++) {
        i++;
        printf("Thread id: %ld, i=%d\n", (long)pthread_self(), i);
        sleep(1);
    }
    return NULL;
}

typedef struct {
    int flag ;
} test;

test * something[4];
__attribute((constructor))
void init()
{
    int i;
    printf("Exec const\n");
    for(i=0;i<4;i++)
    {
        something[i]->flag =0;
    }
    printf("%d\n",something[0]->flag);
}

int main()
{
//    pthread_t tid[5];
//    int b=0;
//    int vals [5];
//    for (b=0; b<5; b++)
//    {
//        vals[b] = b*b;
//        printf("Before calling thread:%d\n",vals[b]);
//        pthread_create(&tid[b], NULL, thread_func,&vals[b]);
//    }
//
//    for (b=0; b<5; b++)
//        pthread_join(tid[b], NULL);
//    return 0;


    printf("%d\n",something[0]->flag);
    return 0;
}