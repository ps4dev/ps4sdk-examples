#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h> // Must be manually handled for handlers as needed
#include <math.h>

#include <kernel.h>
#include <ps4/internal/resolve.h> // used for fail demonstation
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

PS4ResolveStatus errorhandler(char *moduleName, char *symbolName, int *module, void **symbol, PS4ResolveStatus state)
{
	printf("error: %s %s %p %i %p %p %i\n",
		moduleName, symbolName, module, *module, symbol, *symbol, state);
	return state;
}

PS4ResolveStatus prehandler(char *moduleName, char *symbolName, int *module, void **symbol, PS4ResolveStatus state)
{
	printf("pre: %s %s %p %i %p %p %i\n",
		moduleName, symbolName, module, *module, symbol, *symbol, state);
	if(strcmp(symbolName, "abs") == 0) // we fail on abs resolution / call, the return of rax will be undefined
		return PS4ResolveStatusInterceptFailure;
	return state;
}

PS4ResolveStatus posthandler(char *moduleName, char *symbolName, int *module, void **symbol, PS4ResolveStatus state)
{
	printf("post: %s %s %p %i %p %p %i\n",
		moduleName, symbolName, module, *module, symbol, *symbol, state);
	if(strcmp(symbolName, "sceKernelSleep") == 0) // hook sleep
		hookSceKernelSleep((SceKernelSleep *)symbol, hookedSceKernelSleep);
	return state;
}

int main(int argc, char **argv)
{
	int r;
	PS4ResolveStatus stat;

	int module = 0;
	void *symbol = NULL;

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
	printf("%i // should be -14 or so - resolve of main fails (not a libps4 fn)\n", stat);

	printf("%i // result of call to already resolved sleep\n", sceKernelSleep(0));
	stat = ps4Resolve((void *)sceKernelSleep); // handers called, we also hook it
	printf("%i // should be 0 - re-resolve of sceKernelSleep succeeds - also hooked\n", stat);
	printf("%i // result of call to hooked sleep\n", sceKernelSleep(2));

	// failing symbol lookup
	printf("%i // return of failed foo lookup \n", ps4ResolveModuleAndSymbol("libkernel.sprx", "foo", &module, &symbol));

	div_t d = div(128, (int)pow(3.1415, 3));
	r = abs(d.quot + d.rem);
	sleep(1);
	printf("%i // undefined rax return of intercepted noped-out abs call\n", r);
	printf("End\n");

	return EXIT_SUCCESS;
}
