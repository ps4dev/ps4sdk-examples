#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <signal.h>
#include <setjmp.h>

#include <unistd.h>

#include <pthread.h>
#include <errno.h>

/*
	__PS4__ is defined in the Makefile
	We can compile and run this poc under Linux, FreeBSD and the PS4
	Linux is not explicitly checked (used as default)
*/
#ifdef __PS4__
#include <kernel.h>
#include <internal/resolve.h>

FILE *__stdinp;
FILE **__stdinp_addr;
FILE *__stdoutp;
FILE **__stdoutp_addr;
FILE *__stderrp;
FILE **__stderrp_addr;
int __isthreaded;
int *__isthreaded_addr;
#endif

void stdIORedirect(int to, int stdfd[3], fpos_t stdpos[3])
{
	int stdid[3] = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};
	FILE *stdf[3] = {stdin, stdout, stderr};
	int i;

	for(i = 0; i < 3; ++i)
	{
		fflush(stdf[i]);
		fgetpos(stdf[i], &stdpos[i]);

		stdfd[i] = dup(stdid[i]);
		close(stdid[i]);
		dup(to);

		clearerr(stdf[i]);
		setbuf(stdf[i], NULL);
	}
}

void stdIOReset(int stdfd[3], fpos_t stdpos[3])
{
	int stdid[3] = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};
	FILE *stdf[3] = {stdin, stdout, stderr};
	int i;

	for(i = 0; i < 3; ++i)
	{
		fflush(stdf[i]);

		close(stdid[i]);
		dup(stdfd[i]);
		close(stdfd[i]);

		fsetpos(stdf[i], &stdpos[i]);
		clearerr(stdf[i]);
	}
}

void *start_routine(void *arg)
{
	while(*(int *)arg == 1)
	{
		printf("thread> %"PRIxPTR"\n", &errno);
		sleep(1);
	}

	return NULL;
}

#ifdef __PS4__
int64_t _main(void)
#else
int main(int argc, char **argv)
#endif
{
	int server, client;
	struct sockaddr_in serverAddress, clientAddress;
	char message[256];
	int stdfd[3];
	fpos_t stdpos[3];

	pthread_t thread;

	#ifdef __PS4__
	int libc = sceKernelLoadStartModule("libSceLibcInternal.sprx", 0, NULL, 0, 0, 0);
	sceKernelDlsym(libc, "__stdinp", (void **)&__stdinp_addr);
	sceKernelDlsym(libc, "__stdoutp", (void **)&__stdoutp_addr);
	sceKernelDlsym(libc, "__stderrp", (void **)&__stderrp_addr);
	sceKernelDlsym(libc, "__isthreaded", (void **)&__isthreaded_addr);
	__stdinp = *__stdinp_addr;
	__stdoutp = *__stdoutp_addr;
	__stderrp = *__stderrp_addr;
	__isthreaded = *__isthreaded_addr;
	#endif

	memset(&serverAddress, 0, sizeof(serverAddress));
	#ifdef __FreeBSD__ //parent of our __PS4__
	serverAddress.sin_len = sizeof(serverAddress);
	#endif
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(9025);

	memset(&clientAddress, 0, sizeof(clientAddress));

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server < 0)
		return EXIT_FAILURE;

	if(bind(server, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		close(server);
		return EXIT_FAILURE;
	}

	if(listen(server, 10) < 0)
	{
		close(server);
		return EXIT_FAILURE;
	}

	while(1)
	{
		volatile int run = 1;
		client = accept(server, NULL, NULL);

		if(client < 0)
			continue;

		stdIORedirect(client, stdfd, stdpos);

		pthread_create(&thread, NULL, start_routine, (void *)&run);
		scanf("%s", message); // block until input
		printf("main> %"PRIxPTR"\n", &errno);
		run = 0;
		pthread_join(thread, NULL);

		stdIOReset(stdfd, stdpos);

		close(client);
	}

	//close(server);
}
