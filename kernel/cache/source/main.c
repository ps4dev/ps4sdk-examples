#include <stdlib.h>
#include <stdio.h>

#include <ps4/kernel.h>

#define SERVER_PORT 5088

int main(int argc, char **argv)
{
	Ps4KernelCache *c;
	void *m, *m2;
	int r;

	Ps4KernelThread *td;
	Ps4KernelSocket *client;

	if(ps4KernelIsInKernel() != PS4_OK)
	{
		printf("This is not a user space application.\n");
		return PS4_ERROR_IS_KERNEL_ELF;
	}

	ps4KernelThreadGetCurrent(&td);
	r = ps4KernelSocketTCPServerCreateAcceptThenDestroy(td, &client, SERVER_PORT);

	m = m2 = NULL;
	r = ps4KernelCacheCreate(&c);
	r = ps4KernelSocketPrint(td, client, "cache create: %p %i\n", c, r);

	r = ps4KernelCacheGet(c, "foo", &m2);
	r = ps4KernelSocketPrint(td, client, "cache get foo m2: %p %i\n", m2, r);

	m = (void *)42;
	r = ps4KernelCacheSet(c, "foo", m);
	r = ps4KernelSocketPrint(td, client, "cache set foo m: %p %i\n", m, r);

	r = ps4KernelCacheGet(c, "foo", &m2);
	r = ps4KernelSocketPrint(td, client, "cache get foo m2: %p %i\n", m2, r);

	r = ps4KernelCacheDelete(c, "foo");
	r = ps4KernelSocketPrint(td, client, "cache delete foo: %i\n", r);

	r = ps4KernelCacheDelete(c, "bar");
	r = ps4KernelSocketPrint(td, client, "cache delete bar: %i\n", r);

	m2 = NULL;
	r = ps4KernelCacheGet(c, "foo", &m2);
	r = ps4KernelSocketPrint(td, client, "cache get foo m2: %p %i\n", m2, r);

	r = ps4KernelCacheDestroy(c);
	r = ps4KernelSocketPrint(td, client, "cache destroy: %p %i\n", m2, r);

	// Global (rerun)

	m = NULL;
	r = ps4KernelCacheGlobalGet("ps4.test.foo", &m);
	r = ps4KernelSocketPrint(td, client, "m: %p %i\n", m, r);

	m = (void *)0x42;
	r = ps4KernelCacheGlobalSet("ps4.test.foo", m);
	r = ps4KernelSocketPrint(td, client, "m: %p %i\n", m, r);

	m = NULL;
	r = ps4KernelCacheGlobalGet("ps4.test.foo", &m);
	r = ps4KernelSocketPrint(td, client, "m: %p %i\n", m, r);

	//m = ps4KernelDlSym("sysctl_handle_64");
	//r = ps4KernelSocketPrint(td, client, "ps4KernelDlSym sysctl_handle_64: %p\n", m);

	// uses a sub-cache called ps4.kernel.symbol.lookup
	r = ps4KernelSymbolLookUp("sysctl_handle_64", &m);
	r = ps4KernelSocketPrint(td, client, "ps4KernelSymbolLookUp sysctl_handle_64: %p %i\n", m, r);

	m = NULL;
	r = ps4KernelCacheGlobalGet("ps4.kernel.symbol.lookup", &m);
	r = ps4KernelSocketPrint(td, client, "ps4.kernel.symbol.lookup: %p %i\n", m, r);


	r = ps4KernelSocketDestroy(client);

	return EXIT_SUCCESS;
}
