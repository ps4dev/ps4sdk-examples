#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <inttypes.h>

#include <unistd.h>

#include <kernel.h>

#define NAME_SIZE 128

int main(int argc, char **argv)
{
	int i, r;

	char moduleName[NAME_SIZE];
	char symbolName[NAME_SIZE];
	int module;
	void *symbol;

	// prevent const ro.data array
	char *m1 = "libSceLibcInternal.sprx";
	char *m2 = "libkernel.sprx";
	char *m3 = "librt.sprx";
	char *m4 = "libC.sprx";
	char *modules[] = {m1, m2, m3, m4};

	printf("Input: <module name> <symbol name>\n");
	printf("Output: <module name> <symbol name> <module id> <symbol>\n");
	printf("Exit Input: exit -\n");

	while(1)
	{
		moduleName[0] = '\0';
		symbolName[0] = '\0';
		module = 0;
		symbol = NULL;

		if(scanf("%s %s", moduleName, symbolName) < 2)
			break;

		if(strncmp(moduleName, "exit", 4) == 0)
			break;

		module = sceKernelLoadStartModule(moduleName, 0, NULL, 0, NULL, NULL);
		r = sceKernelDlsym(module, symbolName, &symbol);

		// fuzzy search if no resolve was possible
		for(i = 0; i < sizeof(modules) / sizeof(modules[0]) && r != 0; ++i)
		{
			if(strcmp(modules[i], moduleName) == 0)
				continue;
			module = sceKernelLoadStartModule(modules[i], 0, NULL, 0, NULL, NULL);
			r = sceKernelDlsym(module, symbolName, &symbol);
			if(r == 0)
				strcpy(moduleName, modules[i]);
		}

		if(r != 0)
		{
			strcpy(moduleName, "-");
			strcpy(symbolName, "-");
			module = 0;
			symbol = NULL;
		}

		if(printf("%s %s %i %"PRIxPTR"\n", moduleName, symbolName, module, symbol) < 0)
			break;
	}

	return EXIT_SUCCESS;
}
