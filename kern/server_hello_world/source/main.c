#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1
#define _KERNEL
#define _STANDALONE
#define _WANT_UCRED
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
//#include <sys/libkern.h>
#include <sys/systm.h>
#include <sys/syscallsubr.h>

#include <ps4/kern.h>

#define SERVER_PORT 5057

int main(int argc, char **argv)
{
	// send this elf to 5054, connect, see connection get closed
	// send this elf to 5055, close browser, connect, see connection get closed ^^
	struct thread *td;
	int client;

	td = ps4KernThreadCurrent();
	client = ps4KernUtilServerCreateSingleAccept(td, SERVER_PORT);
	ps4KernUtilSocketPrint(td, client, "Hello world from your ps4 kernel, ps4sdk and hito <3\n{main:%p, argc:%i, argv[0]:%s}\n", main, argc, argv[0]);
	ps4KernUtilSocketClose(td, client);

	return 0;
}
