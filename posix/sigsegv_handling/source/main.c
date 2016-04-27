#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <signal.h>
#include <setjmp.h>
#include <pthread.h>

static sigjmp_buf jmpbuf;

void handler(int sig)
{
	write(1, "SIGSEGV\n", 8);
	siglongjmp(jmpbuf, 1);
}

int main(int argc, char **argv)
{
	char message[256];

	struct sigaction action;

	action.sa_handler = handler;
	action.sa_flags = 0; // SA_NODEFER; /* repeated throws are caught */
	sigemptyset(&action.sa_mask);
	sigaction(SIGSEGV, &action, NULL); // setting a handler may do harm to other threads

	printf("loop\n");
	sigsetjmp(jmpbuf, 1);
	printf("oops\n");
	scanf("%s", message); // wait for input
	if(memcmp(message, "exit", 4))
		return EXIT_FAILURE;
	*((char *)0x42) = 42; /* provoke SIGSEGV */

	//signal(SIGSEGV, SIG_DFL); // reset may harm other "elfs" in multi-thread mode

	return EXIT_SUCCESS;
}
