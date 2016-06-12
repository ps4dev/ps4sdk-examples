#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ps4/memory.h>

int main(int argc, char **argv)
{
	 //shm interface to keep stable memory between reloads, if needed
	Ps4MemoryShared *foo, *bar, *baz;
	void *foom;
	void *barm;
	void *bazm;
	size_t s;


	ps4MemorySharedOpen(&foo, 0x6000 * 3, "/foo");
	ps4MemorySharedOpen(&bar, 0x5000 * 2, "/bar");
	ps4MemorySharedOpen(&baz, 0, "/foo"); // reopen

	printf("Ps4MemoryShared *foo %p\n", foo);
	printf("Ps4MemoryShared *bar %p\n", bar);
	printf("Ps4MemoryShared *baz %p\n", baz);

	ps4MemorySharedGetSize(foo, &s);
	printf("ps4MemorySharedSize *foo %p\n", s);
	ps4MemorySharedGetSize(bar, &s);
	printf("ps4MemorySharedSize *bar %p\n", s);
	ps4MemorySharedGetSize(baz, &s);
	printf("ps4MemorySharedSize *baz %p\n", s);

	ps4MemorySharedGetAddress(foo, &foom);
 	ps4MemorySharedGetAddress(bar, &barm);
 	ps4MemorySharedGetAddress(baz, &bazm);

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

	ps4MemorySharedClose(bar); // leave /bar data
	ps4MemorySharedOpen(&bar, 0x4000 * 6, "/bar");
	printf("Ps4MemoryShared *bar %p\n", bar);
	ps4MemorySharedGetSize(bar, &s);
	printf("ps4MemorySharedSize *bar %p\n", s);
	ps4MemorySharedGetAddress(bar, &barm);
	printf("void *barm %p\n", barm);
	printf("%s\n", barm); // /bar data still there
	strcpy(barm, "I am yyy");
	printf("%s\n", barm);
	ps4MemorySharedUnlink(bar); // remove /bar data

	ps4MemorySharedClose(foo); // leave data
	ps4MemorySharedOpen(&bar, 0, "/foo"); // open new alias
	printf("Ps4MemoryShared *bar %p\n", bar);
	ps4MemorySharedGetSize(bar, &s);
	printf("ps4MemorySharedSize *bar %p\n", s);
	ps4MemorySharedGetAddress(bar, barm);
	printf("void *barm %p\n", barm);
	printf("%s\n", barm); // /foo data still there in "bar" struct
	strcpy(barm, "I am xxx");
	printf("%s\n", barm);
	ps4MemorySharedUnlink(bar); // remove /foo data

	printf("void *bazm %p\n", bazm);
	printf("%s\n", bazm);
	ps4MemorySharedUnlink(baz); // remove /baz data

	return EXIT_SUCCESS;
}
