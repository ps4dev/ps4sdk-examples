#include <stdlib.h>

#include <ps4/memory.h>

// see posix/shm_open example
int main(int argc, char **argv)
{
	Ps4MemoryProtected *pm;
	unsigned char *w;
	unsigned char *e;
	int i;

	ps4MemoryProtectedCreate(&pm, 1234);
	ps4MemoryProtectedGetWritableAddress(pm, (void **)&w);
	ps4MemoryProtectedGetExecutableAddress(pm, (void **)&e);

	for(i = 0; i < 1234; i += 2)
	{
		w[i] = 0xeb; // jump
		w[i + 1] = 0xfe; // to self (loop infinitely)
	}

	// jump somewhere "even" => infinite jump loop
	void (*run)(void) = (void (*)(void))(e + 8);
	run();

	// Never
	ps4MemoryProtectedDestroy(pm);

	return EXIT_SUCCESS;
}
