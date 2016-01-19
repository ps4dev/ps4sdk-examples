#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __PS4__
#include <kernel.h>

__typeof__(__stdoutp) __stdoutp;
__typeof__(__stdoutp) *__stdoutp_address;
__typeof__(__mb_sb_limit) __mb_sb_limit;
__typeof__(__mb_sb_limit) *__mb_sb_limit_address;
__typeof__(_CurrentRuneLocale) _CurrentRuneLocale;
__typeof__(_CurrentRuneLocale) *_CurrentRuneLocale_address;
#endif

#define DirectoryBufferSize 1024 * 1024
#define TemporaryNameSize 4096

int printDirectoryTree_(const char *name, int depth)
{
	int dir, count;
	char *buffer;
	char *temporaryName;
    struct dirent *entry;
	int position;
	struct stat sb;

	dir = open(name, O_RDONLY | O_DIRECTORY);
	if(dir == -1)
		goto e1;

	buffer = malloc(DirectoryBufferSize);
	if(buffer == NULL)
		goto e2;

	temporaryName = malloc(TemporaryNameSize);
	if(temporaryName == NULL)
		goto e3;

	count = getdents(dir, buffer, DirectoryBufferSize);
	if(count <= 0)
		goto e4;

	entry = (struct dirent *)buffer;
	if(entry == NULL || entry->d_reclen == 0)
		goto e4;

	for(position = 0; position < count;)
	{
		entry = (struct dirent *)(buffer + position);

		if(entry->d_reclen == 0)
			break;

		if(strncmp(entry->d_name, ".", entry->d_namlen) == 0 ||	strncmp(entry->d_name, "..", entry->d_namlen) == 0)
		{
			position += entry->d_reclen;
			continue;
		}

		strcpy(temporaryName, name);
		{
			size_t l = strlen(name);
			if(l > 0 && name[l - 1] != '/')
				strcat(temporaryName, "/");
		}
		strcat(temporaryName, entry->d_name);
		printf("%s", temporaryName);
		if(stat(temporaryName, &sb) >= 0)
		{
			printf(" [size: %"PRIdMAX" mode: ", (intmax_t)sb.st_size);
			printf((S_ISDIR(sb.st_mode)) ? "d" : "-");
			printf((sb.st_mode & S_IRUSR) ? "r" : "-");
			printf((sb.st_mode & S_IWUSR) ? "w" : "-");
			printf((sb.st_mode & S_IXUSR) ? "x" : "-");
			printf((sb.st_mode & S_IRGRP) ? "r" : "-");
			printf((sb.st_mode & S_IWGRP) ? "w" : "-");
			printf((sb.st_mode & S_IXGRP) ? "x" : "-");
			printf((sb.st_mode & S_IROTH) ? "r" : "-");
			printf((sb.st_mode & S_IWOTH) ? "w" : "-");
			printf((sb.st_mode & S_IXOTH) ? "x" : "-");
			printf("]");
		}
		printf("\n");

		if(entry->d_type == DT_UNKNOWN)
		{
			position += entry->d_reclen;
			continue;
		}

		if(entry->d_type == DT_DIR && entry->d_fileno != 0)
			printDirectoryTree_(temporaryName, depth + 1);

		fflush(stdout);

		position += entry->d_reclen;
	}

	free(temporaryName);
	free(buffer);
	close(dir);

	return 0;

	e4:
		free(temporaryName);
	e3:
		free(buffer);
	e2:
		close(dir);
	e1:

	return -1;
}

int printDirectoryTree(const char *name)
{
	struct stat sb;

	printf("%s", name);
	if(stat(name, &sb) >= 0)
	{
		printf(" [size: %"PRIdMAX" mode: ", (intmax_t)sb.st_size);
		printf((S_ISDIR(sb.st_mode)) ? "d" : "-");
		printf((sb.st_mode & S_IRUSR) ? "r" : "-");
		printf((sb.st_mode & S_IWUSR) ? "w" : "-");
		printf((sb.st_mode & S_IXUSR) ? "x" : "-");
		printf((sb.st_mode & S_IRGRP) ? "r" : "-");
		printf((sb.st_mode & S_IWGRP) ? "w" : "-");
		printf((sb.st_mode & S_IXGRP) ? "x" : "-");
		printf((sb.st_mode & S_IROTH) ? "r" : "-");
		printf((sb.st_mode & S_IWOTH) ? "w" : "-");
		printf((sb.st_mode & S_IXOTH) ? "x" : "-");
		printf("]");
	}
	printf("\n");
	return printDirectoryTree_(name, 0);
}

int main(int argc, char **argv)
{
	#ifdef __PS4__
	int libc = sceKernelLoadStartModule("libSceLibcInternal.sprx", 0, NULL, 0, 0, 0);
	sceKernelDlsym(libc, "__stdoutp", (void **)&__stdoutp_address);
	__stdoutp = *__stdoutp_address;
	sceKernelDlsym(libc, "__mb_sb_limit", (void **)&__mb_sb_limit_address);
	__mb_sb_limit = *__mb_sb_limit_address;
	sceKernelDlsym(libc, "_CurrentRuneLocale", (void **)&_CurrentRuneLocale_address);
	_CurrentRuneLocale = *_CurrentRuneLocale_address;
	#endif

	printDirectoryTree("/");
	fflush(stdout);

	return EXIT_SUCCESS;
}
