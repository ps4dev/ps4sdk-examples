
#include "common.h"
#include "util.h"

int printBytes(unsigned char *str, size_t size)
{
	int i;
	if(str == NULL)
		return -1;
	for(i = 0; i < size; ++i)
		printf("%02X", (unsigned char)str[i]);
	printf("\n");
	return 0;
}

int printPrintableBytes(unsigned char *str, size_t size)
{
	int i;
	if(str == NULL)
		return -1;
	for(i = 0; i < size; ++i)
		if(isprint(str[i]))
			printf("%c", str[i]);
		else
			printf(" ");
	printf("\n");
	return 0;
}

int utilServerCreate(int port, int backlog, int try, unsigned int sec)
{
	int server;
	struct sockaddr_in serverAddress;
	int r;

	memset(&serverAddress, 0, sizeof(serverAddress));
	#ifdef __FreeBSD__ //parent of our __PS4__
	serverAddress.sin_len = sizeof(serverAddress);
	#endif
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);

	for(; try > 0; --try)
	{
		server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(server < 0)
			sleep(sec);
	}

	if(server < 0)
		return -1;

	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char *)&(int){ 1 }, sizeof(int));
	setsockopt(server, SOL_SOCKET, SO_REUSEPORT, (char *)&(int){ 1 }, sizeof(int));

	if((r = bind(server, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
	{
		close(server);
		return -2;
	}

	if((r = listen(server, backlog)) < 0)
	{
		close(server);
		return -3;
	}

	return server;
}

int utilSingleAcceptServer(int port)
{
	int server, client;
	if((server = utilServerCreate(port, 1, 20, 1)) < 0)
		return server;
	client = accept(server, NULL, NULL); // either return is fine
	close(server);
	return client;
}
