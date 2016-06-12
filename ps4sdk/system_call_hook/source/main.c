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

char *getRandomizedPath()
{
	char path[16];
	int length = 11;
	char *r;
	// on 1.75 using path alone will null out (override) the first 4 bytes
 	// return probably a two value struct with val1 = null
	syscall(SYS_randomized_path, 0, path + 4, &length);
	r = malloc(12);
	r[0] = '/';
	strcpy(r, path + 4);
	return r;
}

void printHook(Ps4KernelSystemCallHook *h)
{
	Ps4KernelSystemCallHookArgument a;
	Ps4KernelSystemCallHookArgument *arg = &a;

 	ps4KernelCall(ps4KernelMemoryCopy, h, arg, sizeof(Ps4KernelSystemCallHookArgument));

	printf(
		"hook[%p] =\n"
		"{\n"
			"\tclone: %p,\n"
			"\thandler: %p,\n"
			"\tprologue: %p,\n"
			"\thook: %p,\n"
			"\thookType: %p,\n"
			"\thookCount: %p,\n"
			"\thookSize: %p,\n"
			"\thookTypeCurrent: %p,\n"
			"\tlock: %p,\n"
			"\tentryCount: %p,\n"
			"\tcallCount: %p,\n"
			"\tsystemCalls: %p,\n"
			"\tnumber: %p,\n"
			"\toriginalCall: %p,\n"
			"\tthread: %p,\n"
			"\tuap: %p,\n"
			"\tsysret: %p,\n"
			"\treturns[%p]:\n"
 			"\t{\n"
				"\t\t0: %p,\n"
				"\t\t1: %p,\n"
 			"\t}\n"
			"\tuserArgument: %p,\n"
			"\tallocate: %p,\n"
			"\tfree: %p,\n"
			"\tmt: %p\n"
		"}\n",
		arg,
		arg->clone,
		arg->handler,
		arg->prologue,
		arg->hook,
		arg->hookType,
		arg->hookCount,
		arg->hookSize,
		arg->hookTypeCurrent,
		arg->lock,
		arg->entryCount,
		arg->callCount,
		arg->systemCalls,
		arg->number,
		arg->originalCall,
		arg->thread,
		arg->uap,
		arg->sysret,
		arg->returns,
		arg->returns[0],
		arg->returns[1],
		arg->userArgument,
		arg->allocate,
		arg->free,
		arg->mt
	);

	//for(int i = 0; i < arg->hookCount; ++i)
	//	printf("hook[%i]: %p %p\n", i, arg->hook[i], arg->hookType[i]);
}

int main(int argc, char **argv)
{
	void *hook;
	int r;

	r = ps4KernelExecute((void *)kmain, NULL, (void *)&hook, NULL);

	char *p = getRandomizedPath();
	printf("%s\n", p);
	printf("%p\n", hook);
	printHook(hook);

	r = ps4KernelExecute((void *)kmain, (void *)hook, NULL, NULL);

	return EXIT_SUCCESS;
}
