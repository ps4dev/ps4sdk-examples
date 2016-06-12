#define _KERNEL

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/libkern.h>

#include <sys/syscall.h>
#include <sys/sysproto.h>

#include <ps4/kernel.h>

#include "kmain.h"

typedef struct Ps4RandomizedPathHookArgument
{
	uint64_t zero;
	void *path;
	uint64_t length;
}
Ps4RandomizedPathHookArgument;

int ps4RandomizedPathHook(struct thread *td, Ps4KernelSystemCallHookArgument *uap)
{
	memcpy(((Ps4RandomizedPathHookArgument *)uap->uap)->path, "mrfoo\0", 6);
	return PS4_KERNEL_SYSTEM_CALL_HOOK_CONTROL_CONTINUE;
}

int kmain(struct thread *td, void *uap)
{
	Ps4KernelSystemCallHook *h = NULL;

	if(uap)
	{
		ps4KernelSystemCallHookDestroy((Ps4KernelSystemCallHook *)uap);
	}
	else
	{
		ps4KernelSystemCallHookCreate(&h, SYS_randomized_path);
		ps4KernelSystemCallHookAdd(h, (void *)ps4RandomizedPathHook, PS4_KERNEL_SYSTEM_CALL_HOOK_TYPE_GENERIC_POST);
	}

	ps4KernelThreadSetReturn(td, (register_t)h);
	return PS4_OK;
}
