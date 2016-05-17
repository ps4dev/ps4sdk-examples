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

#undef offsetof
#include <ps4/kernel.h>
#include <ps4/kern.h>

#define SERVER_PORT 5057
#define SERVER_BACKLOG 10

int main(int argc, char **argv)
{
	// send this elf to 5054, connect, see connection get closed
	// send this elf to 5055, close browser, connect, see connection get closed ^^
	struct thread *td = ps4KernThreadCurrent();
	int client = ps4KernUtilServerCreateSingleAccept(td, SERVER_PORT); //other versions exits
	kern_close(td, client);
	return 0;
}
