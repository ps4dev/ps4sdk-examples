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

#include <ps4/standard_io.h>
#include <ps4/kernel.h>

#include "kmain.h"

int main(int argc, char **argv)
{
	void *sceSblACMgrIsVideoplayerProcess;
	char *mem;
	int64_t ret;
	int r;

	printf("uid: %zu\n", getuid());
	ps4KernelCall(ps4KernelPrivilegeEscalate);
	printf("uid: %zu\n", getuid());

	sceSblACMgrIsVideoplayerProcess = (void *)ps4KernelCall(ps4KernelDlSym, "sceSblACMgrIsVideoplayerProcess");

	mem = malloc(64);
	memset(mem, 0x90, 64);
	strcpy(mem, "Hello World!");

	printf("mem: %p: %s\n", mem, mem);
	ps4StandardIoPrintHexDump(mem, 48);
	r = ps4KernelExecute((void *)kmain1, mem, &ret, NULL);
	printf("mem: %p: %s\n", mem, mem);
	ps4StandardIoPrintHexDump(mem, 48);
	printf("[K1] r: %i, ret: %"PRIxPTR"\n", r, ret);

	ps4KernelCall(ps4KernelMemoryCopy, sceSblACMgrIsVideoplayerProcess, mem, 32);
	ps4StandardIoPrintHexDump(mem, 48);

	r = ps4KernelExecute((void *)kmain2, mem, &ret, NULL);
	printf("[K2] r: %i, ret: %"PRIxPTR"\n", r, ret);

	ps4KernelCall(ps4KernelMemoryCopy, sceSblACMgrIsVideoplayerProcess, mem, 32);
	ps4StandardIoPrintHexDump(mem, 48);

	r = ps4KernelExecute((void *)kmain3, mem, &ret, NULL);
	printf("[K3] r: %i, ret: %"PRIxPTR"\n", r, ret);

	ps4KernelCall(ps4KernelMemoryCopy, sceSblACMgrIsVideoplayerProcess, mem, 32);
	ps4StandardIoPrintHexDump(mem, 48);

	free(mem);

	return EXIT_SUCCESS;
}
