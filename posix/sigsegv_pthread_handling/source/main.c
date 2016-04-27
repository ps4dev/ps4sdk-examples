#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <signal.h>
#include <setjmp.h>
#include <pthread.h>
#include <errno.h>

/*
	__PS4__ is defined in the Makefile
	We can compile and run this poc under Linux, FreeBSD and the PS4
	Linux is not explicitly checked (used as default)
*/
#ifdef __PS4__
#include <kernel.h>

FILE *__stdinp;
FILE **__stdinp_addr;
FILE *__stdoutp;
FILE **__stdoutp_addr;
FILE *__stderrp;
FILE **__stderrp_addr;
int __isthreaded;
int *__isthreaded_addr;
#endif

/* Constants */
enum{ ThreadCount = 32 };

/* Types */
typedef struct
{
	unsigned int id;
	int *errnoAddress;
	sigjmp_buf threadJump;
	pthread_t thread;
}
ThreadData;

static ThreadData threadData[ThreadCount];

int ThreadCreate(unsigned int id, void *(*f)(void *))
{
	if(id >= ThreadCount)// || threadData[id].id != 0)
		return 0;
	printf("ThreadCreate %i %p\n", id, f);
	fflush(stdout);

	threadData[id].id = id;
	pthread_create(&threadData[id].thread, NULL, f, &threadData[id]);

	return 1;
}

void sigsegvHandler(int sig)
{
	volatile int i;

	if(sig != SIGSEGV)
		return;

	//write(1, "SIGSEGV\n", 8);
	for(i = 0; i < ThreadCount; ++i)
		if(threadData[i].errnoAddress == &errno)
			siglongjmp(threadData[i].threadJump, 1);
}

void *t1(void *data)
{
	ThreadData *td = (ThreadData *)data;
	td->errnoAddress = &errno;

	printf("t1 %p %p\n", td, &errno);
	fflush(stdout);

	if(sigsetjmp(td->threadJump, 1) == 0)
	{
		// do
		*(char *)0x42 = 42;
	}
	//while (1);

	return NULL;
}

void *t2(void *data)
{
	ThreadData *td = (ThreadData *)data;
	td->errnoAddress = &errno;

	printf("t2 %p %p\n", td, &errno);
	fflush(stdout);

	if(sigsetjmp(td->threadJump, 1) == 0)
	{
		// do
		uint32_t i;
		for(i = 0; i < (uint32_t) - 1 / 4; ++i)
			;
		*(char *)0x42 = 42;
	}

	return NULL;
}

int main(int argc, char **argv)
{
	struct sigaction action;

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

	action.sa_handler = sigsegvHandler;
	action.sa_flags = 0; // SA_NODEFER; /* repeated throws are caught */
	sigemptyset(&action.sa_mask);
	sigaction(SIGSEGV, &action, NULL);

	ThreadCreate(0, t1);
	ThreadCreate(1, t2);

	pthread_join(threadData[0].thread, NULL);
	pthread_join(threadData[1].thread, NULL);

	return EXIT_SUCCESS;
}
