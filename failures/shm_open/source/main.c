#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifndef MAP_TYPE
	#define MAP_TYPE 0x0f
#endif

int main(int argc, char **argv)
{
	long pz;
 	int h, h2;
	char *w, *e;
	int i;

	pz = sysconf(_SC_PAGESIZE);
	h = shm_open("/jmploop", O_CREAT|O_TRUNC|O_RDWR, 0755);
	h2 = dup(h); // could have mattered ;)
	ftruncate(h, pz);

	#ifdef __PS4__ // neither works on the ps4
		w = mmap(NULL, pz, PROT_READ | PROT_WRITE, MAP_SHARED, h, 0);
		e = mmap(NULL, pz, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_TYPE, h2, 0);
	#else
		w = mmap(NULL, pz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, h, 0);
		e = mmap(NULL, pz, PROT_READ | PROT_EXEC, MAP_FILE | MAP_SHARED, h2, 0);
	#endif

	close(h);
	close(h2);

	for(i = 0; i < pz; i += 2)
	{
		w[i] = 0xeb;
		w[i + 1] = 0xfe;
	}
	w[0] = 0xeb;
	w[1] = 0xfc;

	void (*run)(void) = (void (*)(void))(e + 8);
	run();

	shm_unlink("/jmploop");

	return EXIT_SUCCESS;
}
