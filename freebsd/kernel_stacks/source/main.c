#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <unistd.h>
#include <signal.h>
//#include <setjmp.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/syscall.h>

#include "uthash.h"

#ifdef __PS4__
#include <kernel.h>

__typeof__(__stdoutp) __stdoutp;
__typeof__(__stdoutp) *__stdoutp_address;
__typeof__(__mb_sb_limit) __mb_sb_limit;
__typeof__(__mb_sb_limit) *__mb_sb_limit_address;
__typeof__(_CurrentRuneLocale) _CurrentRuneLocale;
__typeof__(_CurrentRuneLocale) *_CurrentRuneLocale_address;

#endif

typedef struct KernelFunction
{
	uintmax_t id; // address
	int number; // seen first at
	char found[700]; // seen at
	char name[256]; // name
	UT_hash_handle hh;
}
KernelFunction;

/*
static sigjmp_buf jmpbuf;

void sighandler(int sig)
{
	siglongjmp(jmpbuf, 1);
}
*/

int printBytes(char *str, size_t size)
{
	int i;
	if(str == NULL)
		return -1;
	for(i = 0; i < size; ++i)
		printf("%02X", (unsigned char)str[i]);
	printf("\n");
	return 0;
}

int printPrintableBytes(char *str, size_t size)
{
	int i;
	if(str == NULL)
		return -1;
	for(i = 0; i < size; ++i)
		if(isprint(str[i]))
			printf("%c", str[i]);
		else
			printf(" ");
	printf("\n");
	return 0;
}

void *loopSyscall(void *arg)
{
	int *syscallNumber = (int *)arg;
	while(*syscallNumber >= 0)
		syscall(*syscallNumber, 0, 0, 0, 0, 0);
	return NULL;
}

void *loopSyscall_rand(void *arg)
{
	int *syscallNumber = (int *)arg;
	while(*syscallNumber >= 0) // rand aint thread safe ... who gives
		syscall(*syscallNumber, rand(), rand(), rand(), rand(), rand());
	return NULL;
}

void hashKernelFunctionAdd(KernelFunction **kernelFunctions, int address, char *name, int number)
{
	KernelFunction *s;

	HASH_FIND_INT(*kernelFunctions, &address, s);
	if (s == NULL)
	{
	    s = (KernelFunction *)calloc(sizeof(KernelFunction), 1);
	    s->id = address;
		s->number = number;
	    strcpy(s->name, name);
	    HASH_ADD_INT(*kernelFunctions, id, s);
	}
	s->found[number] = 1;
}

void hashKernelFunctionPrint(KernelFunction **kernelFunctions)
{
	KernelFunction *s, *t;
	int i;

	HASH_ITER(hh, *kernelFunctions, s, t)
	{
		printf("0x%"PRIxMAX" %s %i [", s->id, s->name, s->number);
		for(i = 0; i < 700; ++i)
			if(s->found[i] == 1)
				printf("%i ", i);
		printf("]\n");
		fflush(stdout);
		HASH_DEL(*kernelFunctions, s);
		free(s);
	}
}

void hashKernelFunctionsFromStackTrace(KernelFunction **kernelFunctions, char *trace, int number)
{
	int index;
	intmax_t address;
	intmax_t offset;
	char *name;

	char plusData[2048];
	char *line, *lineSave;

	//printf("%s", trace);
	//fflush(stdout);

	line = strtok_r(trace, "\n", &lineSave);
	while(line != NULL)
	{
		char *token, *tokenSave, *plus, *plusSave;

		token = strtok_r(line, " ", &tokenSave);
		sscanf(token, "#%i ", &index);
		token = strtok_r(NULL, " ", &tokenSave);
		sscanf(token, "%"SCNxMAX" at ", &address);
		token = strtok_r(NULL, " ", &tokenSave);
		token = strtok_r(NULL, " ", &tokenSave);
		sscanf(token, "%s", plusData);

		name = strtok_r(plusData, "+", &plusSave);
		plus = strtok_r(NULL, "+", &plusSave);
		sscanf(plus, "%"SCNxMAX, &offset);

		line = strtok_r(NULL, "\n", &lineSave);

		//printf("%"PRIxMAX" %s\n", address - offset, name);
		//fflush(stdout);
		hashKernelFunctionAdd(kernelFunctions, address - offset, name, number);
	}
}

int main(int argc, char **argv)
{
	//struct sigaction action;
	pthread_t threads[16];
	struct kinfo_kstack *stacks;
	size_t stackCount;
	int name[4];
	volatile int syscallNumber, i, j, k, l;
	char block[700];

	memset(block, 0, 700);
	block[0] = 1; // SIGSYS, change args
	block[1] = 1;
	block[8] = 1; // "old"
	block[11] = 0; // "old" but doesn't crash?
	block[18] = 1; // "freebsd4"
	block[19] = 1; // "old"
	block[38] = 1; // unchecked but "old"
	block[40] = 1; // unchecked but "old"
	block[44] = 1; // unchecked but "old"
	block[46] = 1; // "old"
	block[48] = 1; // unchecked but "old"
	block[52] = 1; // unchecked but "old"
	block[62] = 1; // unchecked but "old"
	block[63] = 1; // unchecked but "old"
	block[64] = 1; // unchecked but "old"
	block[67] = 1; // unchecked but "old"
	block[68] = 1; // unchecked but "old"
	block[71] = 1; // unchecked but "old"
	block[76] = 1; // unchecked but "old"
	block[77] = 1; // unchecked but "old"
	block[84] = 1; // "old"
	block[87] = 1; // unchecked but "old"
	block[88] = 1; // unchecked but "old"
	block[93] = 1; // blocks - change args ... ?
	block[99] = 1; // unchecked but "old"
	block[101] = 1; // unchecked but "old"
	block[102] = 1; // unchecked but "old"
	block[103] = 1; // unchecked but "old"
	block[107] = 1; // unchecked but "old"
	block[108] = 1; // unchecked but "old"
	block[109] = 1; // unchecked but "old"
	block[110] = 1; // unchecked but "old"
	block[111] = 1; // unchecked but "old"
	block[112] = 1; // unchecked but "old"
	block[113] = 1; // unchecked but "old"
	block[114] = 1; // unchecked but "old"
	block[115] = 1; // unchecked but "old"
	block[125] = 1; // unchecked but "old"
	block[129] = 1; // unchecked but "old"
	block[130] = 1; // unchecked but "old"
	block[139] = 1; // unchecked but "old"
	block[141] = 1; // unchecked but "old"
	block[142] = 1; // unchecked but "old"
	block[143] = 1; // unchecked but "old"
	block[144] = 1; // unchecked but "old"
	block[145] = 1; // unchecked but "old"
	block[146] = 1; // unchecked but "old"
	block[149] = 1; // unchecked but "old"
	block[150] = 1; // unchecked but "old"
	block[156] = 1; // unchecked but "old"
	block[157] = 1; // "freebsd4"
	block[158] = 1; // "freebsd4"
	block[162] = 1; // "freebsd4"
	block[163] = 1; // "freebsd4"
	block[164] = 1; // "freebsd4"
	block[198] = 1; // SIGSYS, change args
	block[297] = 1; // unchecked but "freebsd4"
	block[313] = 1; // unchecked but "old"
	block[322] = 1; // unchecked but "old"
	block[323] = 1; // unchecked but "old"
	block[336] = 1; // unchecked but "freebsd4"
	block[342] = 1; // unchecked but "freebsd4"
	block[344] = 1; // unchecked but "freebsd4"
	block[431] = 1; // blocks
	block[442] = 1; // blocks
	block[455] = 1;
	block[578] = 1; // kernel crash
	block[522] = 1; // when run from 0 to 617 - this one hangs ...
 	block[579] = 1; // kernel crash

	srand(time(NULL));

	#ifdef __PS4__
	int libc = sceKernelLoadStartModule("libSceLibcInternal.sprx", 0, NULL, 0, 0, 0);
	sceKernelDlsym(libc, "__stdoutp", (void **)&__stdoutp_address);
	__stdoutp = *__stdoutp_address;
	sceKernelDlsym(libc, "__mb_sb_limit", (void **)&__mb_sb_limit_address);
	__mb_sb_limit = *__mb_sb_limit_address;
	sceKernelDlsym(libc, "_CurrentRuneLocale", (void **)&_CurrentRuneLocale_address);
	_CurrentRuneLocale = *_CurrentRuneLocale_address;
	#endif

	/*
	action.sa_handler = sighandler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGSYS, &action, NULL);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	*/

	KernelFunction *kernelFunctions = NULL;

	name[0] = CTL_KERN;
	name[1] = KERN_PROC;
	name[2] = KERN_PROC_KSTACK;
	name[3] = getpid();

	for(k = 0; k <= 617; ++k)
	//for(k = 531; k <= 617; ++k)
	{
		/*
		if(sigsetjmp(jmpbuf, 1) != 0)
		{
			//printf("SIGSEGV, SIGILL or SIGSYS -> skipping %i\n", k);
			printf("<-error ");
			continue;
		}
		*/

		printf("%i ", k);

		if(block[k] == 1)
		{
			printf("<-skip ");
			if(k % 16 == 0)
				printf("\n");
			continue;
		}

		if(k % 16 == 0)
			printf("\n");

		syscallNumber = k;

		for(l = 0; l < sizeof(threads) / sizeof(threads[0]); ++l)
			pthread_create(&threads[l], NULL, loopSyscall_rand, (void *)&syscallNumber);

		for(i = 0; i < 256; ++i)
		{
			if(sysctl(name, 4, NULL, &stackCount, NULL, 0) < 0)
				break;

			stacks = malloc(stackCount);

			if(sysctl(name, 4, stacks, &stackCount, NULL, 0) < 0)
				break;

			for(j = 0; j < stackCount / sizeof(stacks[0]); ++j)
				hashKernelFunctionsFromStackTrace(&kernelFunctions, stacks[j].kkst_trace, syscallNumber);
			//hashKernelFunctionsFromStackTrace(&kernelFunctions, stacks[0].kkst_trace, syscallNumber);

			free(stacks);
		}

		//hashKernelFunctionPrint(&kernelFunctions);

		syscallNumber = -1;
		for(l = 0; l < sizeof(threads) / sizeof(threads[0]); ++l)
			pthread_join(threads[l], NULL);

		//sleep(10);
	}
	printf("\n");

	hashKernelFunctionPrint(&kernelFunctions);

	return EXIT_SUCCESS;
}
