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
#include <ps4/internal/kernelexploit.h>

#include "kmain.h"

// kernel functions called, in user-land function ... yay for evil voodoo Oo?!
int kmain(int argc, char **argv)
{
	char *moo = argv[0];

	struct malloc_type *mt = ps4KernelDlSym("M_TEMP");
	// please tell me how to get executable memory :P
	void *kmoo = malloc(32, mt, M_ZERO | M_WAITOK);

	// moo, don't go to the dark place ...
	copyin(moo, kmoo, 32);
	strcpy(kmoo, "Cows go to kernel too ...");
	copyout(kmoo, moo, 32);

	free(kmoo, mt);

	return 42; // important notice from kernel!
}
