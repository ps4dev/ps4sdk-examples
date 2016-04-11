#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <ps4/kernel.h>
#include <ps4/internal/asmpayload.h>

#include "kmain.h"

int main(int argc, char **argv)
{
	int kargc = 1;
	char *kargv[2];
	char *moo = malloc(32); // A moo!! :D
	int i;

	kargv[0] = moo;
	kargv[1] = NULL;

	printf("uid: %zu\n", getuid());
	// this syscall returns 0 after the first ps4KernelRun (see in a rerun process)
	// do not use this directly (just for show and tell here)
	// use the self-patching ps4KernelRun wrapper instead
	printf("sys: %i\n", syscall(SYS_ps4_kernel_run, NULL));

	strcpy(moo, "Hmm ... ? *yum, grass*");
	int r = ps4KernelRun(kmain, kargc, kargv);
	printf("return (sceSblACMgrIsVideoplayerProcess): %i\n", r);
	printf("moo: %s\n", moo);

	printf("uid: %zu\n", getuid());
	printf("sys: %i\n", syscall(SYS_ps4_kernel_run, NULL));

	printf("ps4KernelIsInKernel(): %i\n", ps4KernelIsInKernel());
	printf("ps4KernelDlSym(kernel_map): %p\n", ps4KernelDlSym("kernel_map"));
	printf("ps4KernelDlSym(copyin): %p\n", ps4KernelDlSym("copyin"));

	ps4KernelEscalatePrivileges();
	printf("uid: %zu\n", getuid());

	//ps4KernelUARTEnable();

	// and some patching
	memset(moo, '\0', 32);
	void *sceSblACMgrIsVideoplayerProcess = ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	ps4KernelMemcpy(moo, sceSblACMgrIsVideoplayerProcess, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	r = ps4KernelRun(kmain2, kargc, kargv);
	printf("return2 (sceSblACMgrIsVideoplayerProcess): %i\n", r);

	ps4KernelMemcpy(moo, sceSblACMgrIsVideoplayerProcess, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	r = ps4KernelRun(kmain3, kargc, kargv);
	printf("return3 (sceSblACMgrIsVideoplayerProcess): %i\n", r);

	ps4KernelMemcpy(moo, sceSblACMgrIsVideoplayerProcess, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	free(moo); //Bye moo, you did real good :(~

	return EXIT_SUCCESS;
}
