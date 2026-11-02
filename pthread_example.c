#include <pthread.h>
#include <stdio.h>

void    *define_yourself(void *arg)
{
    while (1) {
        printf("I'm the PISOX thread\n");
    }
}

int main(void)
{
    pthread_t   thread;

    printf("I'm the main thread\n");
    pthread_create(&thread, NULL, define_yourself, NULL);
    //pthread_join(thread, NULL);
    printf("There is two threads\n");
}
