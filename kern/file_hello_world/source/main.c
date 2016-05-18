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

#define	STDIN_FILENO 0

int main(int argc, char **argv)
{
	// send this elf to 5054, to see a print to stdout
	// send this elf to 5055, you will see nothing because the kernel stdout is not (yet?) redirected
	struct thread *td;
	int r;

	td = ps4KernThreadCurrent();
	r = ps4KernUtilFilePrint(td, STDIN_FILENO, "Hello world from your ps4 kernel, ps4sdk and hito <3\n{main:%p, argc:%i, argv[0]:%s}\n", main, argc, argv[0]);

	return r;
}
