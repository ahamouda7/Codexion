#include <pthread.h>
#include <stdio.h>

void    *define_yourself(void *arg)
{
    int     i;
    int     *count;

    count = (int *)arg;
    for (i = 0; i < 500; i++) {
        printf("%d\n", *count);
        (*count)++;
    }
}

int main(void)
{
    pthread_t   thread1;
    pthread_t   thread2;
    int         count;

    count = 0;
    pthread_create(&thread1, NULL, define_yourself, &count);
    pthread_create(&thread2, NULL, define_yourself, &count);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Program finished and the new value of count is %d\n", count);
}
