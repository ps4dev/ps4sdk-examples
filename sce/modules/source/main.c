#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <kernel.h>
//#include <video_out.h>

enum{ ModulesSize = 1024 };

void printModuleList(SceKernelModule *modules, size_t size)
{
	int i;

	printf("SceKernelModule[%zu] = {", size);
	for(i = 0; i < size; ++i)
		printf("%02X,", modules[i]);
	printf("}\n");
}


void printModuleInfo(SceKernelModuleInfo *moduleInfo)
{
	int i;

	printf("struct SceKernelModuleInfo\n");
	printf("{\n");
	printf("\tname: %s\n", moduleInfo->name);
	for(i = 0; i < moduleInfo->segmentCount; ++i)
	{
		printf("\t\tsegment[%i].address: %p\n", i, moduleInfo->segmentInfo[i].address);
		printf("\t\tsegment[%i].size: %u\n", i, moduleInfo->segmentInfo[i].size);
		printf("\t\tsegment[%i].prot: %i\n", i, moduleInfo->segmentInfo[i].prot);
	}
	printf("\tsegment count: %u\n", moduleInfo->segmentCount);
	printf("\tfingerprint: ");
	for(i = 0; i < 20; ++i)
		printf("%02X", moduleInfo->fingerprint[i]);
	printf("\n");
	printf("}\n");
}

void stopUnloadModule(SceKernelModule module)
{
	int r;
	SceKernelModule modules[ModulesSize];
	size_t moduleCount;

 	r = sceKernelStopUnloadModule(module, 0, NULL, 0, NULL, NULL);
	printf("sceKernelStopUnloadModule(%02X) = %i\n", module, r);

	r = sceKernelGetModuleList(modules, ModulesSize, &moduleCount);
	printf("sceKernelGetModuleList = %i\n", r);
	printModuleList(modules, moduleCount);
}

void loadStartModule(const char *moduleName)
{
	int r;
	SceKernelModule module;
	SceKernelModule modules[ModulesSize];
	size_t moduleCount;

	module = sceKernelLoadStartModule(moduleName, 0, NULL, 0, NULL, NULL);
	printf("sceKernelLoadStartModule(%s) = %02X\n", moduleName, module);

	r = sceKernelGetModuleList(modules, ModulesSize, &moduleCount);
	printf("sceKernelGetModuleList = %i\n", r);
	printModuleList(modules, moduleCount);
}

int main(int argc, char **argv)
{
	SceKernelModule modules[ModulesSize];
	size_t moduleCount;
	SceKernelModuleInfo *moduleInfo;
	int r, i;

	moduleCount = 0;
 	r = sceKernelGetModuleList(modules, ModulesSize, &moduleCount);
	printf("sceKernelGetModuleList = %i\n", r);
	printModuleList(modules, moduleCount);

	moduleInfo = malloc(moduleCount * sizeof(SceKernelModuleInfo));

	for(i = 0; i < moduleCount; ++i)
	{
		moduleInfo[i].size = sizeof(SceKernelModuleInfo);
		r = sceKernelGetModuleInfo(modules[i], &moduleInfo[i]);
		printf("sceKernelGetModuleInfo(%02X) = %i\n", modules[i], r);
		printModuleInfo(&moduleInfo[i]);
	}

	// Try to unload to free resources like video to reinitialize them
	// Video (open) did still not work though
	// Crashes on return to the browser (obviously)
	// Also crashes on unload of libSceWebKit2 imidiately
	// which could maybe be cought though.

	/*
	for(i = 0; i < moduleCount; ++i)
		if(strcmp(moduleInfo[i].name, "libSceWebKit2.sprx"))
			stopUnloadModule(modules[i]);

	// try to free potential dependency survivors
	for(i = 0; i < moduleCount; ++i)
		if(strcmp(moduleInfo[i].name, "libSceWebKit2.sprx"))
			stopUnloadModule(modules[i]);

	for(i = 0; i < moduleCount; ++i)
		loadStartModule(moduleInfo[i].name);

	free(moduleInfo);
	*/

	return EXIT_SUCCESS;
}
