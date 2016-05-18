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

#include <ps4/kern.h>

#define SERVER_PORT 5057

#ifndef STDOUT_FILENO
	#define	STDOUT_FILENO 1
#endif

static int c;

//int printf(const char *fmt, ...)
int printf_hook(void *td, Ps4KernFunctionHookArgument *uap)
{
	// fully variadic maybe on the next season of ps4sdk
	ps4KernUtilSocketPrint(td, c, (const char *)uap->arguments->general[1]);
	return PS4_KERN_FUNCTION_HOOK_CONTINUE;
}

int main(int argc, char **argv)
{
	// send this elf to 5054, connect, see connection get closed
	// send this elf to 5055, close browser, connect, see connection get closed ^^
	struct thread *td;
	int client;

	td = ps4KernThreadCurrent();
	client = ps4KernUtilServerCreateSingleAccept(td, SERVER_PORT); // see kern/util for more

	ps4KernUtilSocketPrint(td, client, "Hello world from your ps4 kernel and ps4sdk.\n{main:%p, argc:%i, argv[0]:%s}\n\n", main, argc, argv[0]);

	//(you will need to (re)connect 5052 if you send this to 5054)
	//(without messing with the redirected, the rest works as intended)
	//(basically you hijack the user fd and the userland starts interfering)
	//ps4KernUtilStandardIoRedirectPlain(td, client);
	//ps4KernUtilFilePrint(td, STDOUT_FILENO, "Somebody is watching over us... controlling us. It's true, I tell you. It's true! We are merely sprites that dance at the beck and call of our button-pressing overlord. This is a video game. Don't you see? We are characters in a video game!\n\n");

	c = client;
	printf("S.T.A.R.S ... !\n"); // the kernel printf does not go to an fd
	ps4KernFunctionHookSized((void *)ps4KernDlSym("printf"), (void *)printf_hook, 16); // 16 is a valid instruction offset in printf !!!
	printf("It's a weapon. It's really powerful, especially against living things.\n\n");
	ps4KernFunctionUnhook((void *)ps4KernDlSym("printf"));
	printf("S.T.A.R.S ... !\n");

	ps4KernUtilSocketPrint(td, client, "Kind regards and greetings to all who read this <3,\nhito\n");

	ps4KernUtilSocketClose(td, client);

	return 0;
}
