#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ps4/memory.h>

int main(int argc, char **argv)
{
	 //shm interface to keep stable memory between reloads, if needed
	Ps4MemoryShared *foo, *bar, *baz;
	ps4MemorySharedOpen(&foo, "/foo", 0x6000 * 3);
	ps4MemorySharedOpen(&bar, "/bar", 0x5000 * 2);
	ps4MemorySharedOpen(&baz, "/foo", 0); // reopen

	printf("Ps4MemoryShared *foo %p\n", foo);
	printf("Ps4MemoryShared *bar %p\n", bar);
	printf("Ps4MemoryShared *baz %p\n", baz);

	printf("ps4MemorySharedSize *foo %p\n", ps4MemorySharedGetSize(foo));
	printf("ps4MemorySharedSize *bar %p\n", ps4MemorySharedGetSize(bar));
	printf("ps4MemorySharedSize *baz %p\n", ps4MemorySharedGetSize(baz));

	void *foom = ps4MemorySharedGetAddress(foo);
	void *barm = ps4MemorySharedGetAddress(bar);
	void *bazm = ps4MemorySharedGetAddress(baz);

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
	ps4MemorySharedOpen(&bar, "/bar", 0x4000 * 6);
	printf("Ps4MemoryShared *bar %p\n", bar);
	printf("ps4MemorySharedSize *bar %p\n", ps4MemorySharedGetSize(bar));
	barm = ps4MemorySharedGetAddress(bar);
	printf("void *barm %p\n", barm);
	printf("%s\n", barm); // /bar data still there
	strcpy(barm, "I am yyy");
	printf("%s\n", barm);
	ps4MemorySharedUnlink(bar); // remove /bar data

	ps4MemorySharedClose(foo); // leave data
	ps4MemorySharedOpen(&bar, "/foo", 0); // open new alias
	printf("Ps4MemoryShared *bar %p\n", bar);
	printf("ps4MemorySharedSize *bar %p\n", ps4MemorySharedGetSize(bar));
	barm = ps4MemorySharedGetAddress(bar);
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
