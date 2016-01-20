#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <net.h>

int main(int argc, char **argv)
{
	int j, base, current;
	char name[256];

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		current = open("/dev/null", O_RDWR);
		close(current);
	}
	if(base > current - 10)
		printf("open + close Failed, fd did not increase as claimed\n");
	else
		printf("open + close Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		current = socket(AF_INET, SOCK_STREAM, 0);
		close(current);
	}
	if(base > current - 10)
		printf("socket + close Failed, fd did not increase as claimed\n");
	else
		printf("socket + close Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		sprintf(name, "sceNetSocket%i", j);
		current = sceNetSocket(name, AF_INET, SOCK_STREAM, 0);
		close(current);
	}
	if(base > current - 10)
		printf("sceNetSocket + close Failed, fd did not increase as claimed\n");
	else
		printf("sceNetSocket + close Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		sprintf(name, "sceNetSocket%i", j);
		current = sceNetSocket(name, AF_INET, SOCK_STREAM, 0);
		sceNetSocketClose(current);
	}
	if(base > current - 10)
		printf("sceNetSocket + sceNetSocketClose Failed, fd did not increase as claimed\n");
	else
		printf("sceNetSocket + sceNetSocketClose Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		current = sceNetSocket("sceNetSocketWhoCares", AF_INET, SOCK_STREAM, 0);
		close(current);
	}
	if(base > current - 10)
		printf("sceNetSocket + static name + close Failed, fd did not increase as claimed\n");
	else
		printf("sceNetSocket + static name + close Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		current = sceNetSocket("sceNetSocketWhoCares", AF_INET, SOCK_STREAM, 0);
		sceNetSocketClose(current);
	}
	if(base > current - 10)
		printf("sceNetSocket + static name + sceNetSocketClose Failed, fd did not increase as claimed\n");
	else
		printf("sceNetSocket + static name + sceNetSocketClose Success, base: %i, fd: %i\n", base, current);
	close(base);

	base = open("/dev/null", O_RDWR);
	for(j = 0; j < 1024; ++j)
	{
		current = socket(AF_INET, SOCK_STREAM, 0);
		sceNetSocketClose(current);
	}
	if(base > current - 10)
		printf("socket + sceNetSocketClose Failed, fd did not increase as claimed\n");
	else
		printf("socket + sceNetSocketClose Success, base: %i, fd: %i\n", base, current);
	close(base);

	return EXIT_SUCCESS;
}
