#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;

void    *define_yourself(void *arg)
{
    int     i;
    int     *count;

    count = (int *)arg;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < 2000000; i++) {
        printf("%d\n", *count);
        (*count)++;
    }
    pthread_mutex_unlock(&mutex);
    return (NULL);
}

int main(void)
{
    pthread_t   thread1;
    pthread_t   thread2;
    int         count;

    count = 0;
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&thread1, NULL, define_yourself, &count);
    pthread_create(&thread2, NULL, define_yourself, &count);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex);
    printf("Program finished and the new value of count is %d\n", count);
}
