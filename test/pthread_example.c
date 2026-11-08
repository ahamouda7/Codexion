#include <pthread.h>
#include <stdio.h>
 
void    *define_yourself(void *arg)
{
	int     i;
	// int     *id;
	int     count;

	count = *(int *)arg;
	// id = (int *)arg;
	for (i = 0; i < 10; i++) {
		// printf("I'm a PISOX thread, my id is %d\n", *id);
		printf("%d\n", count);
		count++;
	}
	// return id;
}

int main(void)
{
	pthread_t   thread1;
	pthread_t   thread2;
	// int         id1;
	// int         id2;
	void        *a;
	void        *b;
	int         count;

	// id1 = 1;
	// id2 = 2;
	count = 0;
	// printf("I'm the main thread\n");
	pthread_create(&thread1, NULL, define_yourself, &count);
	pthread_create(&thread2, NULL, define_yourself, &count);
	pthread_join(thread1, &a);
	pthread_join(thread2, &b);
	// printf("Pthread 1's ID: %d\n", *(int *)a);
	// printf("Pthread 2's ID: %d\n", *(int *)b);
	// printf("There is two threads\n");
}
