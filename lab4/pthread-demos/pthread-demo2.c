//
//  main.c
//  pthread-demo2
//
//

#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS     6

void *PrintHelloWorld(void *threadid)
{
    int tid;
    tid = *(int *)threadid;
    //for( int i=0; i<5; i++)
    printf("Hello World! Thread ID= %d\n", tid);
    pthread_exit(NULL);
}

int main ()
{
    pthread_t threads[NUM_THREADS];
    int ret, i;
    for( i=0; i < NUM_THREADS; i++ ){
        printf("main() : creating thread, %d\n ", i);
        ret = pthread_create(&threads[i], NULL, PrintHelloWorld, (void *)&i);
        if (ret){
            printf("Error:unable to create thread %d\n", ret);
            return -1;
        }
    }
    pthread_exit(NULL);
}

