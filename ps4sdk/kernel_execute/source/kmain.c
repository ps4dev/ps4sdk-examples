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

#include "kmain.h"

// kernel functions called, in user-land function ... yay for evil voodoo Oo?!
int kmain1(struct thread *td, void *uap)
{
	char *mem = (char *)uap;

	struct malloc_type *mt = ps4KernelDlSym("M_TEMP");

	void *kmem = malloc(32, mt, M_ZERO | M_WAITOK);

	copyin(mem, kmem, 32);
	strcpy(kmem, "Goodbye, cruel world.");
	copyout(kmem, mem, 32);

	free(kmem, mt);

	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	ps4KernelThreadSetReturn(td, sceSblACMgrIsVideoplayerProcess(td)); //see kmain2's content & return

	return EINVAL;
}

int kmain2(struct thread *td, void *uap)
{
	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	ps4KernelFunctionPatchToReturn((void *)sceSblACMgrIsVideoplayerProcess, 0);

	ps4KernelThreadSetReturn(td, sceSblACMgrIsVideoplayerProcess(td)); // important notice from kernel!
	return 0;
}

int kmain3(struct thread *td, void *uap)
{
	RunnableInt sceSblACMgrIsVideoplayerProcess = (RunnableInt)ps4KernelDlSym("sceSblACMgrIsVideoplayerProcess");
	void *sceSblACMgrIsShellcoreProcess = ps4KernelDlSym("sceSblACMgrIsShellcoreProcess");
	ps4KernelFunctionPatchToJump((void *)sceSblACMgrIsVideoplayerProcess, sceSblACMgrIsShellcoreProcess);

	ps4KernelThreadSetReturn(td, sceSblACMgrIsVideoplayerProcess(td));
	return 0;
}
