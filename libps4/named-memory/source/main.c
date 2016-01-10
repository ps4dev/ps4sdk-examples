#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ps4/namedmemory.h>

int main(int argc, char **argv)
{
	 //shm interface to keep stable memory between reloads, if needed
	PS4NamedMemory *foo = ps4NamedMemoryOpen("/foo", 0x6000 * 3);
	PS4NamedMemory *bar = ps4NamedMemoryOpen("/bar", 0x5000 * 2);
	PS4NamedMemory *baz = ps4NamedMemoryOpen("/foo", 0); // reopen

	printf("PS4NamedMemory *foo %p\n", foo);
	printf("PS4NamedMemory *bar %p\n", bar);
	printf("PS4NamedMemory *baz %p\n", baz);

	printf("ps4NamedMemorySize *foo %p\n", ps4NamedMemorySize(foo));
	printf("ps4NamedMemorySize *bar %p\n", ps4NamedMemorySize(bar));
	printf("ps4NamedMemorySize *baz %p\n",ps4NamedMemorySize(baz));

	void *foom = ps4NamedMemoryMemory(foo);
	void *barm = ps4NamedMemoryMemory(bar);
	void *bazm = ps4NamedMemoryMemory(baz);

	printf("%s\n", foom);
	printf("%s\n", barm);
	printf("%s\n", bazm);

	printf("void *foom %p\n", foom);
	printf("void *barm %p\n", barm);
	printf("void *bazm %p\n", bazm);

	strcpy(foom, "I am foo");
	strcpy(barm, "I am bar");
	strcpy(bazm, "I am baz");

	printf("%s\n", foom);
	printf("%s\n", barm);
	printf("%s\n", bazm);

	ps4NamedMemoryClose(bar); // leave /bar data
	bar = ps4NamedMemoryOpen("/bar", 0x4000 * 6);
	printf("PS4NamedMemory *bar %p\n", bar);
	printf("ps4NamedMemorySize *bar %p\n", ps4NamedMemorySize(bar));
	barm = ps4NamedMemoryMemory(bar);
	printf("void *barm %p\n", barm);
	printf("%s\n", barm); // /bar data still there
	strcpy(barm, "I am yyy");
	printf("%s\n", barm);
	ps4NamedMemoryUnlink(bar); // remove /bar data

	ps4NamedMemoryClose(foo); // leave data
	bar = ps4NamedMemoryOpen("/foo", 0); // open new alias
	printf("PS4NamedMemory *bar %p\n", bar);
	printf("ps4NamedMemorySize *bar %p\n", ps4NamedMemorySize(bar));
	barm = ps4NamedMemoryMemory(bar);
	printf("void *barm %p\n", barm);
	printf("%s\n", barm); // /foo data still there in "bar" struct
	strcpy(barm, "I am xxx");
	printf("%s\n", barm);
	ps4NamedMemoryUnlink(bar); // remove /foo data

	printf("void *bazm %p\n", bazm);
	printf("%s\n", bazm);
	ps4NamedMemoryUnlink(baz); // remove /baz data

	return EXIT_SUCCESS;
}
