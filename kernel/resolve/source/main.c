#define __BSD_VISIBLE 1
#define _KERNEL

#include <stdint.h>
#include <inttypes.h>

#include <sys/param.h>
#include <sys/systm.h>

#include <ps4/kernel.h>

#define SERVER_PORT 5088
#define NAME_SIZE 128

int main(int argc, char **argv)
{
	Ps4KernelThread *td;
	Ps4KernelSocket *client;

	char symbolName[NAME_SIZE];
	void *symbol;
	int match;

	ps4KernelThreadGetCurrent(&td);

	ps4KernelSocketTCPServerCreateAcceptThenDestroy(td, &client, SERVER_PORT);

	ps4KernelSocketPrint(td, client, "Input: <symbol name>\n");
	ps4KernelSocketPrint(td, client, "Output: <symbol name> <symbol address>\n");
	ps4KernelSocketPrint(td, client, "To exit input: \"exit!\"\n");

	while(1)
	{
		symbolName[0] = '\0';
		symbol = NULL;

		ps4KernelSocketScan(td, client, &match, "%s", symbolName);
		if(match < 1)
			break;

		if(strncmp(symbolName, "exit!", 4) == 0)
			break;

		symbol = ps4KernelDlSym(symbolName);

		if(ps4KernelSocketPrint(td, client, "%s %"PRIxPTR"\n", symbolName, (uintptr_t)symbol) != PS4_OK)
			break;
	}

	ps4KernelSocketPrint(td, client, "Done\n");

	ps4KernelSocketDestroy(client);

	return PS4_OK;
}
