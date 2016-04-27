#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

char *sceKernelGetRandomizedPath()
{
	char path[16];
	int length = 11;
	char *r;
	// on 1.75 using path alone will null out (override) the first 4 bytes
 	// thus, return probably a two value struct with val1 = null
	syscall(602, 0, path + 4, &length);
	r = malloc(12);
	r[0] = '/';
	strcpy(r, path + 4);
	return r;
}

int main(int argc, char **argv)
{
	char path[1024];
	char *args[2];
	int r;

	strcpy(path, "/");
	strcat(path, sceKernelGetRandomizedPath());
	// the current procress seems to be able to exec itself
	// nothing else ...
	// so the runtime seems to be restricted this way
	strcat(path, "/common/lib/WebProcess.self");

	args[0] = path;
	args[1] = NULL; // Invalid args ...

	r = execv(args[0], args);
    printf("execv(%s) = %i\n", args[0], r);

	return EXIT_SUCCESS;
}
