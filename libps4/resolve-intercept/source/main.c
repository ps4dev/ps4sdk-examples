#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <pthread.h> //FIXME: Needs to be implemented
#include <math.h>

#include <kernel.h>
#include <ps4/internal/resolve.h> // used for fail demonstation
#include <ps4/resolve.h>

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
	return state;
}

int main(int argc, char **argv)
{
	int r;
	PS4ResolveStatus stat;

	int module = 0;
	void *symbol = NULL;

	//ps4Resolve((void *)strcmp); // either this or a call
	// we need to resolve (or call) strcmp and printf once or run into an infinite loop on a resolve intercept
	if(strcmp("foo", "bar") != 0)
		printf("Start\n");

	ps4Resolve((void *)sceKernelSleep); // handers not called, fn resolved

	ps4ResolveSetErrorHandler(errorhandler);
	ps4ResolveSetPreHandler(prehandler);
	ps4ResolveSetPostHandler(posthandler);

	stat = ps4Resolve((void *)sceKernelUsleep); // handlers called, fn resolved
	printf("%i // should be 0 - resolve of sceKernelUsleep succeeds\n", stat);

	stat = ps4Resolve((void *)main); // check should fail, nothing resolved
	printf("%i // should be -14 or so - resolve of main fails (not a libps4 fn)\n", stat);

	// failing symbol lookup
	printf("%i // return of failed foo lookup \n", ps4ResolveModuleAndSymbol("libkernel.sprx", "foo", &module, &symbol));

	div_t d = div(128, (int)pow(3.1415, 3));
	r = abs(d.quot + d.rem);
	sleep(1);
	printf("%i // error rax return of intercepted noped-out abs call\n", r);
	printf("End\n");

	return EXIT_SUCCESS;
}
