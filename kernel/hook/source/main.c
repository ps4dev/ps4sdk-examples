#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

//#include <sys/sysproto.h>

#include <ps4/kernel.h>
#include <ps4/kern.h>
#include <ps4/payload.h>

#include <float.h>

// auto root on jit ^^'
int hook1(void *td, Ps4KernFunctionHookArgument *uap)
{
	//ps4KernPrivilegeRoot();
	__asm__ volatile(" \
		movq %gs:0, %rax; \n \
		mov 0x8(%rax),%rax \n \
		mov 0x40(%rax),%rax \n \
		movl $0x0,0x14(%rax) \n \
		movq $0x0,0x4(%rax) \n \
	");
	//uap->arguments->rax = 0; // return OK
	//return PS4_KERN_FUNCTION_HOOK_RETURN; // intercept call
	return PS4_KERN_FUNCTION_HOOK_CONTINUE; // next hook
}

int hook2(void *td, Ps4KernFunctionHookArgument *uap)
{
	// unroot
	uap->arguments->rax = 0; // return OK
	return PS4_KERN_FUNCTION_HOOK_RETURN; // intercept call
}

void printHook(Ps4KernelFunctionHook *h)
{
	Ps4KernFunctionHookArgument *arg = (Ps4KernFunctionHookArgument *)h;
	Ps4KernFunctionHookArgument a;
	ps4KernelMemoryCopy(arg, &a, sizeof(Ps4KernFunctionHookArgument));
	arg = &a;
	printf("[%p %p %p %p]\n[%p %p %p %p]\n[%p %p %p %p]\n[%p %p %p %p]\n",
		arg->function,
		arg->hook,
		arg->hookCurrent,
		arg->hookCount,
		arg->hookSize,

		arg->bridge,
		arg->bridgeCopiedSize,
		arg->arguments,

		arg->locked,
		arg->reserved1,
		arg->entryCount,
		arg->callCount,

		arg->returnTo,
		arg->r12,
		arg->stage1,
		arg->stage0);
	void *hook[arg->hookSize];
	ps4KernelMemoryCopy(arg->hook, hook, arg->hookSize * sizeof(void *));
	for(int i = 0; i < arg->hookSize; ++i)
		printf("-> %p\n", hook[i]);
}

int main(int argc, char **argv)
{
	void *a = ps4KernelDlSym("sceSblACMgrIsJitApplicationProcess");
	int r = 0;
	size_t s = 0;
	Ps4KernelFunctionHook *hh;
	void *h1, *h2;

 	h1 = ps4KernelMemoryMalloc(128);
	printf("ps4KernelMemoryMalloc: %p\n", h1);
	ps4KernelMemoryCopy((void *)hook1, h1, 128);
	printf("ps4KernelMemoryCopy: %p %p\n", hook1, h1);

 	h2 = ps4KernelMemoryMalloc(128);
	printf("ps4KernelMemoryMalloc: %p\n", h2);
	ps4KernelMemoryCopy((void *)hook2, h2, 128);
	printf("ps4KernelMemoryCopy: %p %p\n", hook2, h2);

	// We need to do this manually for now, since llvm code
	// does not
	s = 12;
	r = ps4KernelMachineInstructionSeek(a, &s);
	printf("ps4KernelMachineInstructionSeek: %i %zu\n", r, s);

	r = ps4KernelFunctionIsHooked(a);
	printf("ps4KernelFunctionIsHooked: %i %p\n", r, a);

	r = ps4KernelFunctionGetHook(a, &hh);
	printf("ps4KernelFunctionGetHook: %i %p %p\n", r, &hh, hh);

	sleep(2);

	r = ps4KernelFunctionHookCreateSized(&hh, a, h2, s);
	printf("ps4KernelFunctionHookCreateSized: %i %p %p %p %p %zu\n", r, &hh, hh, a, h2, s);

	printHook(hh);
	printf("uid: %zu\n", getuid());
	for(int i = 0; i < 10; ++i)
	{
		syscall(SYS_jitshm_create, 0, 0, 0, 0, 0);
		printf("SYS_jitshm_create\n");
	}
	printHook(hh);
	printf("no root? -> uid: %zu\n", getuid());

	r = ps4KernelFunctionIsHooked(a);
	printf("ps4KernelFunctionIsHooked: %i %p\n", r, a);

	hh = NULL;
	r = ps4KernelFunctionGetHook(a, &hh);
	printf("ps4KernelFunctionGetHook: %i %p %p\n", r, &hh, hh);

	sleep(2);

	r = ps4KernelFunctionHookAdd(hh, h1);
	printf("ps4KernelFunctionHookAdd: %i %p %p %p\n", r, &hh, hh, h1);

	syscall(SYS_jitshm_create, 0, 0, 0, 0, 0);
	printHook(hh);
	printf("no root? -> uid: %zu\n", getuid());

	r = ps4KernelFunctionHookRemove(hh, h2);
	printf("ps4KernelFunctionHookRemove: %i %p %p %p\n", r, &hh, hh, h2);

	syscall(SYS_jitshm_create, 0, 0, 0, 0, 0);
	printHook(hh);
	printf("root because h1 intercept gone? -> uid: %zu\n", getuid());

	r = ps4KernelFunctionHookAdd(hh, h1);
	printf("ps4KernelFunctionHookAdd: %i %p %p %p\n", r, &hh, hh, h1);

	syscall(SYS_jitshm_create, 0, 0, 0, 0, 0);
	printHook(hh);
	printf("uid: %zu\n", getuid());

	r = ps4KernelFunctionHookAdd(hh, h2);
	printf("ps4KernelFunctionHookAdd: %i %p %p %p\n", r, &hh, hh, h2);

	syscall(SYS_jitshm_create, 0, 0, 0, 0, 0);
	printHook(hh);
	printf("uid: %zu\n", getuid());
	printf("current should be 1 - intercept by first h1\n");

	r = ps4KernelFunctionUnhook(a);
	printf("ps4KernelFunctionUnhook: %i %p\n", r, a);

	ps4KernelMemoryFree(h2);
	ps4KernelMemoryFree(h1);

	return EXIT_SUCCESS;
}
