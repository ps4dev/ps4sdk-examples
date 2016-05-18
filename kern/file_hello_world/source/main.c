#include <ps4/kern.h>

#ifndef STDOUT_FILENO
	#define	STDOUT_FILENO 1
#endif

int main(int argc, char **argv)
{
	// send this elf to 5054, to see a print to stdout
	// send this elf to 5055, you will see nothing because the kernel stdout is not (yet?) redirected
	struct thread *td;
	int r;

	td = ps4KernThreadCurrent();
	r = ps4KernUtilFilePrint(td, STDOUT_FILENO, "Hello world from your ps4 kernel, ps4sdk.\nKind regards hito <3\n\n{main:%p, argc:%i, argv[0]:%s}\n", main, argc, argv[0]);

	return r;
}
