//
//  main.c
//  pthread-demo3
//
//

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>


#define NUM_THREADS     6


void *ThreadWait(void *t)
{
    int threadid;
    
    threadid = (int)t;
    
    printf("Thread %d is sleeping\n", threadid);
    sleep(3);
    printf("Thread %d is exiting\n", threadid);
    pthread_exit(NULL);
}

int main ()
{
    int ret,i;
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    void *status;
    
    // Initialize and set thread attributes to "joinable"
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    for( i=0; i < NUM_THREADS; i++ ){
        printf("main() is creating thread %d \n", i);
        ret = pthread_create(&threads[i], NULL, ThreadWait, (void *)i );
        if (ret){
            printf("Unable to create thread %d\n", i);
            return -1;
        }
    }
    
    // deallocate attribute and wait for the other threads
    pthread_attr_destroy(&attr);
    for( i=0; i < NUM_THREADS; i++ ){
        ret = pthread_join(threads[i], &status);
        if (ret){
            printf("Error:unable to join thread %d, return = %d\n", i, ret);
            return -1;
        }
        printf("Thread %d has completed with status = %d\n", i, (int)status) ;
    }
    
    printf("program is exiting.\n");
    
    pthread_exit(NULL);
}
