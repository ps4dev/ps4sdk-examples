#undef _SYS_CDEFS_H_
#undef _SYS_TYPES_H_
#undef _SYS_PARAM_H_
#undef _SYS_MALLOC_H_

#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1
#define _KERNEL
#define _WANT_UCRED
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#undef offsetof
#include <kernel.h>
#include <ps4/kernel.h>
#include <ps4/kern.h>

#include "kmain.h"

// kernel functions called, in user-land function ... yay for evil voodoo Oo?!
int kmain(struct thread *td, void *uap)
{
	char *moo = (char *)uap;

	struct malloc_type *mt = ps4KernelDlSym("M_TEMP");
	// please tell me how to get executable memory :P
	moo[0] = 'M';
	void *kmoo = malloc(32, mt, M_ZERO | M_WAITOK);

	// moo, don't go to the dark place ...
	copyin(moo, kmoo, 32);
	strcpy(kmoo, "Cows go to kernel too ...");
	copyout(kmoo, moo, 32);

	free(kmoo, mt);

	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernDlSym("sceSblACMgrIsVideoplayerProcess");

	ps4KernThreadSetReturn0(td, sceSblACMgrIsVideoplayerProcess(td)); //see kmain2's content & return
	return EINVAL;
}

int kmain2(struct thread *td, void *uap)
{
	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	ps4KernelFunctionPatchToReturn((void *)sceSblACMgrIsVideoplayerProcess, 0);

	ps4KernThreadSetReturn0(td, sceSblACMgrIsVideoplayerProcess(td)); // important notice from kernel!
	return 0;
}

int kmain3(struct thread *td, void *uap)
{
	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	void *sceSblACMgrIsShellcoreProcess = ps4KernelDlSym("sceSblACMgrIsShellcoreProcess");
	ps4KernelFunctionPatchToJump((void *)sceSblACMgrIsVideoplayerProcess, sceSblACMgrIsShellcoreProcess);

	ps4KernThreadSetReturn0(td, sceSblACMgrIsVideoplayerProcess(td));
	return 0;
}
