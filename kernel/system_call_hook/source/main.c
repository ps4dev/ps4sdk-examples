#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1
#define _KERNEL
#define _STANDALONE
#define _WANT_UCRED
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/param.h>
#include <sys/kernel.h>
//#include <sys/libkern.h>
#include <sys/systm.h>

#include <sys/sysproto.h>
//#include <sys/unistd.h>
#include <sys/syscallsubr.h>
#include <sys/syscall.h>

#include <sys/proc.h>
#include <sys/kthread.h>

#include <ps4/kernel.h>

#define SERVER_PORT 5088

static Ps4KernelSocket *sock;


int sysCloseHook(struct thread *td, struct close_args *uap)
{
	ps4KernelSocketPrint(td, sock, "pre: %i\n", uap->fd);
	return PS4_KERNEL_SYSTEM_CALL_HOOK_CONTROL_CONTINUE;
}

int sysCloseHookGeneric(struct thread *td, Ps4KernelSystemCallHookArgument *arg)
{
	struct close_args *uap;
	uap = (struct close_args *)arg->uap;

	ps4KernelSocketPrint(td, sock, "generic: %i\n", uap->fd);
	return PS4_KERNEL_SYSTEM_CALL_HOOK_CONTROL_CONTINUE;
}

void socketPrintHook(Ps4KernelThread *td, Ps4KernelSocket *s, Ps4KernelSystemCallHook *h)
{
	Ps4KernelSystemCallHookArgument *arg = (Ps4KernelSystemCallHookArgument *)h;

	ps4KernelSocketPrint(td, s,
		"hook[%p] =\n"
		"{\n"
			"\tclone: %p,\n"
			"\thandler: %p,\n"
			"\tprologue: %p,\n"
			"\thook: %p,\n"
			"\thookType: %p,\n"
			"\thookCount: %p,\n"
			"\thookSize: %p,\n"
			"\thookTypeCurrent: (will be null *) %p,\n"
			"\tlock: %p,\n"
			"\tentryCount: %p,\n"
			"\tcallCount: %p,\n"
			"\tsystemCalls: %p,\n"
			"\tnumber: %p,\n"
			"\toriginalCall: %p,\n"
			"\tthread: (will be null *) %p,\n"
			"\tuap: (will be null *) %p,\n"
			"\tsysret: (will be (32bit) null *) %p,\n"
			"\treturns[%p]:\n"
 			"\t{\n"
				"\t\t0: (will be null *) %p,\n"
				"\t\t1: (will be null *) %p,\n"
 			"\t}\n"
			"\tuserArgument: %p,\n"
			"\tallocate: %p,\n"
			"\tfree: %p,\n"
			"\tmt: %p\n"
		"}\n"
		"* = This is will not show per-hook runtime values due to the lock-less design.\n",
		arg,
		arg->clone,
		arg->handler,
		arg->prologue,
		arg->hook,
		arg->hookType,
		arg->hookCount,
		arg->hookSize,
		arg->hookTypeCurrent,
		arg->lock,
		arg->entryCount,
		arg->callCount,
		arg->systemCalls,
		arg->number,
		arg->originalCall,
		arg->thread,
		arg->uap,
		arg->sysret,
		arg->returns,
		arg->returns[0],
		arg->returns[1],
		arg->userArgument,
		arg->allocate,
		arg->free,
		arg->mt
	);

	for(int i = 0; i < arg->hookCount; ++i)
		ps4KernelSocketPrint(td, s, "hook[%i]: %p %p\n", i, arg->hook[i], arg->hookType[i]);
}

int main(int argc, char **argv)
{
	struct thread *td;
	struct socket *client;
	int number;
	Ps4KernelSystemCallHook *h;
    struct sysent *sy;
	int r;

	ps4ExpressionReturnOnError(ps4KernelSymbolLookUp("sysent", (void **)&sy));

	if(ps4KernelIsInKernel() != PS4_OK)
	{
		printf("This is not a user space application.\n");
		return PS4_ERROR_IS_KERNEL_ELF;
	}

	ps4KernelThreadGetCurrent(&td);

	r = ps4KernelSocketTCPServerCreateAcceptThenDestroy(td, &client, SERVER_PORT);

	sock = client;

	// press "options" to see menue handles getting closed
	number = SYS_close;
	r = ps4KernelSocketPrintHexDump(td, client, &sy[number], sizeof(struct sysent));

	ps4KernelSystemCallHookCreate(&h, number);
	r = ps4KernelSocketPrintHexDump(td, client, &sy[number], sizeof(struct sysent));
	ps4KernelSystemCallHookAdd(h, (void *)sysCloseHookGeneric, PS4_KERNEL_SYSTEM_CALL_HOOK_TYPE_GENERIC_BOTH);
	ps4KernelSystemCallHookAdd(h, (void *)sysCloseHook, PS4_KERNEL_SYSTEM_CALL_HOOK_TYPE_PRE);
	pause("hooked", 10000);
	socketPrintHook(td, client, h);
	ps4KernelSystemCallHookDestroy(h);
	r = ps4KernelSocketPrintHexDump(td, client, &sy[number], sizeof(struct sysent));

	r = ps4KernelSocketDestroy(client);

	return PS4_OK;
}
