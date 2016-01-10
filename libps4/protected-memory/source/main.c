#include <stdlib.h>

#include <ps4/protectedmemory.h>

// see posix/shm_open example
int main(int argc, char **argv)
{
	PS4ProtectedMemory *pm = ps4ProtectedMemoryCreate(1234);
	unsigned char *w = ps4ProtectedMemoryWritable(pm);
	unsigned char *e = ps4ProtectedMemoryExecutable(pm);
	int i;

	for(i = 0; i < 1234; i += 2)
	{
		w[i] = 0xeb; // jump
		w[i + 1] = 0xfe; //to w[i - 2]
	}
	w[0] = 0xeb; // jump
	w[1] = 0xfc; // to this (w[0]) loop infinitely

	// jump sled + infinite jump loop
	void (*run)(void) = (void (*)(void))(e + 8);
	run();

	// Never
	ps4ProtectedMemoryDestroy(pm);

	return EXIT_SUCCESS;
}
