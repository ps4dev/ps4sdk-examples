#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <ps4/kernel.h>

#include "kmain.h"

int main(int argc, char **argv)
{
	int kargc = 1;
	char *kargv[2];
	char *moo = malloc(32); // A moo!! :D

	kargv[0] = moo;
	kargv[1] = NULL;

	printf("uid: %zu\n", getuid());
	// this syscall turns to return 0 after the first ps4KernelExecute
	// do not call it directly - use the patching ps4KernelExecute
	printf("sys: %i\n", syscall(SYS_ps4_kernel_execute, NULL));

	strcpy(moo, "Hmm ... ? *yum, grass*");
	int r = ps4KernelRun(kmain, kargc, kargv);
	printf("return: %i\n", r);
	printf("moo: %s\n", moo);
	free(moo); //Bye moo :(

	printf("uid: %zu\n", getuid());
	printf("sys: %i\n", syscall(SYS_ps4_kernel_execute, NULL));

	printf("ps4KernelIsInKernel(): %i\n", ps4KernelIsInKernel());
	printf("ps4KernelDlSym(kernel_map): %p\n", ps4KernelDlSym("kernel_map"));
	printf("ps4KernelDlSym(copyin): %p\n", ps4KernelDlSym("copyin"));

	ps4KernelEscalatePrivileges();
	printf("uid: %zu\n", getuid());

	return EXIT_SUCCESS;
}
