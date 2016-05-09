#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <unistd.h>

#include <sys/syscall.h>

#include <ps4/kernel.h>

#include "kmain.h"

int main(int argc, char **argv)
{
	char *moo = malloc(32); // A moo!! :D
	int i;
	int64_t ret;

	printf("uid: %zu\n", getuid());
	// this syscall returns 0 after the first ps4KernelRunMain (see in a rerun process)
	// do not use this directly (just for show and tell here)
	// use the self-patching ps4KernelRunMain wrapper instead
	printf("sys: %i\n", syscall(SYS_ps4_callback, NULL));

	strcpy(moo, "Hmm ... ? *yum, grass*");
	// I will also add a ps4KernelRun(Syscall?) that uses the syscall interface instead
	int r = ps4KernelExecute((void *)kmain, moo, &ret, NULL);
	printf("return (sceSblACMgrIsVideoplayerProcess): %i %"PRId64"\n", r, ret);
	printf("moo: %s\n", moo);
	printf("moo: %p\n", moo);

	printf("uid: %zu\n", getuid());
	printf("sys: %i\n", syscall(SYS_ps4_callback, NULL));

	printf("ps4KernelIsInKernel(): %i\n", ps4KernelIsInKernel());
	printf("ps4KernelDlSym(kernel_map): %p\n", ps4KernelDlSym("kernel_map"));
	printf("ps4KernelDlSym(copyin): %p\n", ps4KernelDlSym("copyin"));

	ps4KernelPrivilegeEscalate();
	printf("uid: %zu\n", getuid());

	//ps4KernelUARTEnable();

	// and some patching
	memset(moo, '\0', 32);
	void *sceSblACMgrIsVideoplayerProcess = ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	ps4KernelMemoryCopy(sceSblACMgrIsVideoplayerProcess, moo, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	r = ps4KernelExecute((void *)kmain2, moo, &ret, NULL);
	printf("return2 (sceSblACMgrIsVideoplayerProcess): %i %"PRId64"\n", r, ret);

	ps4KernelMemoryCopy(sceSblACMgrIsVideoplayerProcess, moo, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	r = ps4KernelExecute((void *)kmain3, moo, &ret, NULL);
	printf("return3 (sceSblACMgrIsVideoplayerProcess): %i %"PRId64"\n", r, ret);

	ps4KernelMemoryCopy(sceSblACMgrIsVideoplayerProcess, moo, 32);
	for(i = 0; i < 32; ++i)
		printf("%02X", ((unsigned char *)moo)[i]);
	printf("\n");

	free(moo); //Bye moo, you did real good :(~

	return EXIT_SUCCESS;
}
