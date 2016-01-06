#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <unistd.h>

#include <pthread.h>
#include <errno.h>

void *start_routine(void *arg)
{
	while(*(int *)arg == 1)
	{
		printf("thread> %"PRIxPTR"\n", &errno);
		sleep(1);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	char message[256];
	pthread_t thread;
	volatile int run = 1;

	pthread_create(&thread, NULL, start_routine, (void *)&run);
	scanf("%s", message); // block until input
	printf("main> %"PRIxPTR"\n", &errno);
	run = 0;
	pthread_join(thread, NULL);

	return EXIT_SUCCESS;
}
