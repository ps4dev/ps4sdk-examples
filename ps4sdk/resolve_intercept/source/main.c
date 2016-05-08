#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h> // Must be manually handled for handlers as needed
#include <math.h>

#include <kernel.h>
#include <ps4/resolve.h>

typedef unsigned int (*SceKernelSleep)(unsigned int seconds);

SceKernelSleep originalSceKernelSleep;
unsigned int hookedSceKernelSleep(unsigned int seconds)
{
	unsigned int i;
	printf("All your base");
	i = originalSceKernelSleep(seconds);
	printf(" are belong to us.\n");
	return i;
}

void hookSceKernelSleep(SceKernelSleep *symbol, SceKernelSleep fn)
{
	originalSceKernelSleep = *symbol;
	*symbol = fn;
}

PS4ResolveStatus errorhandler(PS4ResolveState *status)
{
	printf("error: %s %s %p %i %p %p %i\n",
		status->module, status->symbol, status->moduleId, *status->moduleId, status->address, *status->address, status->kernelAddress);
	return status->status;
}

PS4ResolveStatus prehandler(PS4ResolveState *status)
{
	printf("pre: %s %s %p %i %p %p %i\n",
		status->module, status->symbol, status->moduleId, *status->moduleId, status->address, *status->address, status->kernelAddress);
	if(strcmp(status->symbol, "abs") == 0) // we fail on abs resolution / call, the return of rax will be undefined
		return PS4ResolveStatusInterceptFailure;
	return status->status;
}

PS4ResolveStatus posthandler(PS4ResolveState *status)
{
	printf("post: %s %s %p %i %p %p %i\n",
		status->module, status->symbol, status->moduleId, *status->moduleId, status->address, *status->address, status->kernelAddress);
	if(strcmp(status->symbol, "sceKernelSleep") == 0) // hook sleep
		hookSceKernelSleep((SceKernelSleep *)status->address, hookedSceKernelSleep);
	return status->status;
}

int main(int argc, char **argv)
{
	uint64_t r = 0;
	PS4ResolveStatus stat;

	// either ps4Resolve or a call before using them in a handler or
	// we run into an infinite loop on a resolve interception
	//ps4Resolve((void *)strcmp);
	//ps4Resolve((void *)printf);
	if(strcmp("foo", "bar") != 0)
		printf("Start\n");

	ps4Resolve((void *)sceKernelSleep); // handers not set & called, fn resolved

	ps4ResolveSetErrorHandler(errorhandler);
	ps4ResolveSetPreHandler(prehandler);
	ps4ResolveSetPostHandler(posthandler);

	stat = ps4Resolve((void *)sceKernelUsleep); // handlers called, fn resolved
	printf("%i // should be 0 - resolve of sceKernelUsleep succeeds\n", stat);

	stat = ps4Resolve((void *)main); // check should fail, nothing resolved
	printf("%i // should be -14 or so - resolve of main fails (not a ps4sdk fn)\n", stat);

	printf("%i // result of call to already resolved sleep\n", sceKernelSleep(0));
	stat = ps4Resolve((void *)sceKernelSleep); // handers called, we also hook it
	printf("%i // should be 0 - re-resolve of sceKernelSleep succeeds - also hooked\n", stat);
	printf("%i // result of call to hooked sleep\n", sceKernelSleep(2));

	div_t d = div(128, (int)pow(3.1415, 3));
	r = abs(d.quot + d.rem);
	sleep(1);
	printf("%i // undefined rax return of intercepted noped-out abs call\n", r);
	printf("End\n");

	return EXIT_SUCCESS;
}
