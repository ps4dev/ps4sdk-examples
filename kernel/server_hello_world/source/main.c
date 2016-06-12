#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <ps4/socket.h>
#include <ps4/stream.h>

#include <ps4/kernel.h>

#define SERVER_PORT 5088

int kernel_main(int argc, char **argv);
int user_main(int argc, char **argv);

int main(int argc, char **argv) // adaptive main
{
	if(ps4KernelIsInKernel() == PS4_OK)
		// send this elf to 5054, connect, see connection get closed
		// send this elf to 5055, close browser, connect, see connection get closed ^^
		return kernel_main(argc, argv);
	// send this elf to 5053, connect, see connection get closed
	return user_main(argc, argv);
}

int kernel_main(int argc, char **argv)
{
	Ps4KernelThread *td;
	Ps4KernelSocket *client;
	int r;

	/*
	struct socket *server;
	struct sockaddr_in address;

	memset(&address, 0, sizeof(address));
	address.sin_len = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(SERVER_PORT);
	*/

	ps4KernelThreadGetCurrent(&td);

	/*
	server = client = NULL;
	r = ps4KernelSocketTCPServerCreate(td, &server, SERVER_PORT, 10);
	r = ps4KernelSocketAccept(td, server, &client);

	r = ps4KernelSocketPrint(td, client, "foo\n");

	r = ps4KernelSocketDestroy(td, client);
	r = ps4KernelSocketDestroy(td, server);
	*/

	r = ps4KernelSocketTCPServerCreateAcceptThenDestroy(td, &client, SERVER_PORT);

	r = ps4KernelSocketPrint(td, client, "Hello world from your ps4 kernel via ps4sdk. Kind regards and greetings to all who read this, hito <3\n{main:%p, argc:%i, argv[0]:%s}\n\n", main, argc, argv[0]);

	r = ps4KernelSocketDestroy(client);

	return PS4_OK;
}

int user_main(int argc, char **argv)
{
	int client;
	FILE *cout;
	//FILE *cin;
	int r;

	r = ps4SocketTCPServerCreateAcceptThenDestroy(&client, SERVER_PORT);
	//ps4StreamOpenFileDuplicate(&cin, client, "rb");
	ps4StreamOpenFileDuplicate(&cout, client, "wb");
	close(client);

	// Change standard io buffering
	//setvbuf(cin, NULL, _IOLBF, 0);
	setvbuf(cout, NULL, _IONBF, 0);
	//setvbuf(cerr, NULL, _IONBF, 0);

	fprintf(cout, "Hello world from your ps4 user space via ps4sdk. Kind regards and greetings to all who read this, hito <3\n{main:%p, argc:%i, argv[0]:%s}\n\n", main, argc, argv[0]);

	fclose(cout);
	//fclose(cin);

	return PS4_OK;
}
