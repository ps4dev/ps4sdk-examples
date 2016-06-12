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

#include <ps4/kernel.h>

#define SERVER_PORT 5088

static Ps4KernelSocket *sock;

int strnlenHook(const char *s, size_t n)
{
	struct thread *td;

	ps4KernelThreadGetCurrent(&td);
	// !!! do not use format => strlen inf. loop => crash !!!
	ps4KernelSocketPrint(td, sock, s);
	ps4KernelSocketPrint(td, sock, "\n");

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int snprintfHook(char *str, size_t size, const char *format, ...)
{
	struct thread *td;
	va_list args;

	ps4KernelThreadGetCurrent(&td);
	// !!! do not use format => strlen inf. loop => crash !!!
	ps4KernelSocketPrint(td, sock, "%p %zu ", str, size);
	va_start(args, format);
	ps4KernelSocketPrintSizedWithArgumentList(td, sock, size, format, args);
	va_end(args);
	ps4KernelSocketPrint(td, sock, "\n");

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int strncpyHook(char *dest, const char *src, size_t n)
{
	struct thread *td;

	ps4KernelThreadGetCurrent(&td);
	ps4KernelSocketPrint(td, sock, "%s\n", src);

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int printfHook(const char *format, ...)
{
	struct thread *td;
	va_list args;

	ps4KernelThreadGetCurrent(&td);
	va_start(args, format);
	ps4KernelSocketPrintWithArgumentList(td, sock, format, args);
	va_end(args);
	ps4KernelSocketPrint(td, sock, "\n");

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int kern_closeHook(struct thread *td, int fd)
{
	ps4KernelSocketPrint(td, sock, "%p %i\n", td, fd);
	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int indexHook(const char *s, int c)
{
	struct thread *td;

	ps4KernelThreadGetCurrent(&td);
	ps4KernelSocketPrint(td, sock, "%s %i %c\n", s, c, c);

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

int genericHook(struct thread *td, Ps4KernelFunctionHookArgument *arg)
{
	ps4KernelSocketPrint(td, sock, "Type %p:\n", arg->hookTypeCurrent);

	ps4KernelSocketPrint(td, sock, "%p(%p, %p, %p, %p, %p, %p) %p\n\t => %p %p %p %p %p %p\n",
		arg->function,
		arg->arguments->rdi, arg->arguments->rsi, arg->arguments->rdx,
		arg->arguments->rcx, arg->arguments->r8, arg->arguments->r9,
		arg->arguments->rax,
		arg->returns->rax, arg->returns->rdx,
		arg->returns->xmm0.low, arg->returns->xmm0.high,
		arg->returns->xmm1.low, arg->returns->xmm1.high
	);

	return PS4_KERNEL_FUNCTION_HOOK_CONTROL_CONTINUE;
}

void socketPrintHook(struct thread *td, Ps4KernelSocket *s, Ps4KernelFunctionHook *h)
{
	Ps4KernelFunctionHookArgument *arg = (Ps4KernelFunctionHookArgument *)h;

	ps4KernelSocketPrint(td, s,
		"hook[%p] =\n"
		"{\n"
			"\tfunction: %p,\n"
			"\thook: %p,\n"
			"\thookCurrent: (will be null *) %p,\n"
			"\thookCount: %p,\n"
			"\thookSize: %p,\n"
			"\tbridge: %p,\n"
			"\tbridgeCopiedSize: %p,\n"
			"\targuments[%p]: (will contain null *) \n"
			"\t{\n"
				"\t\trax: %p,\n"
				"\t\trdi: %p,\n"
				"\t\trsi: %p,\n"
				"\t\trdx: %p,\n"
				"\t\trcx: %p,\n"
				"\t\tr8: %p,\n"
				"\t\tr9: %p\n"
			"\t},\n"
			"\tlock: %p,\n"
			"\thookTypeCurrent: (will be null *) %p,\n"
			"\tentryCount: %p,\n"
			"\tcallCount: %p,\n"
			"\treturnTo: (will be null *) %p,\n"
			"\tr12: (will be null *) %p,\n"
			"\thandler: %p,\n"
			"\tprologue: %p,\n"
			"\thookType: %p,\n"
			"\treturns[%p]: (will contain null *) \n"
 			"\t{\n"
				"\t\trax: %p,\n"
				"\t\trdx: %p,\n"
				"\t\txmm0: %p %p,\n"
				"\t\txmm1: %p %p\n"
 			"\t}\n"
			"\tallocate: %p\n"
			"\tfree: %p\n"
			"\tmt: %p\n"
			"\tuserArgument: %p\n"
		"}\n",
		"* = This is will not show per-hook runtime values due to the lock-less design.\n",
		arg,
		arg->function,
		arg->hook,
		arg->hookCurrent,
		arg->hookCount,
		arg->hookSize,
		arg->bridge,
		arg->bridgeCopiedSize,
		arg->arguments,
		arg->arguments->rax,
		arg->arguments->rdi,
		arg->arguments->rsi,
		arg->arguments->rdx,
		arg->arguments->rcx,
		arg->arguments->r8,
		arg->arguments->r9,
		arg->lock,
		arg->hookTypeCurrent,
		arg->entryCount,
		arg->callCount,
		arg->returnTo,
		arg->r12,
		arg->handler,
		arg->prologue,
		arg->hookType,
		arg->returns,
		arg->returns->rax,
		arg->returns->rdx,
		arg->returns->xmm0.low, arg->returns->xmm0.high,
		arg->returns->xmm1.low, arg->returns->xmm1.high,
		arg->allocate,
		arg->free,
		arg->mt,
		arg->userArgument
	);

	for(int i = 0; i < arg->hookCount; ++i)
		ps4KernelSocketPrint(td, sock, "hook[%i]: %p %p\n", i, arg->hook[i], arg->hookType[i]);
}

#include <stddef.h>
#include <sys/proc.h>
#include <sys/kthread.h>

int main(int argc, char **argv)
{
	struct thread *td;
	struct socket *client;
	void *fn;
	Ps4KernelFunctionHook *h;
	int r;

	if(ps4KernelIsInKernel() != PS4_OK)
	{
		printf("This is not a user space application.\n");
		return PS4_ERROR_IS_KERNEL_ELF;
	}

	ps4KernelThreadGetCurrent(&td);

	r = ps4KernelSocketTCPServerCreateAcceptThenDestroy(td, &client, SERVER_PORT);

	sock = client;

	fn = ps4KernelDlSym("kern_close");
	r = ps4KernelSocketPrintHexDump(td, client, fn, 64);
	ps4KernelFunctionHook(fn, (void *)genericHook, PS4_KERNEL_FUNCTION_HOOK_TYPE_GENERIC_BOTH);
	ps4KernelFunctionPrehook(fn, (void *)kern_closeHook);
	r = ps4KernelSocketPrintHexDump(td, client, fn, 64);
	pause("hooked", 10000);
	ps4KernelFunctionGetHook(fn, &h);
	socketPrintHook(td, client, h);
	ps4KernelFunctionUnhook(fn);
	r = ps4KernelSocketPrintHexDump(td, client, fn, 64);

	r = ps4KernelSocketDestroy(client);

	return PS4_OK;
}
