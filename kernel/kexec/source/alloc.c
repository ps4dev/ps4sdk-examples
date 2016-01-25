#include "common.h"
#include "alloc.h"

static int globalFileDescriptor;

int kexecGenerateFileDescriptor(int number)
{
	int t[KExecFileDescriptorMax];
	int fd, base, diff, i, err;

	fd = -1;
	err = 0;

	//base = open("/dev/null", O_RDWR);
	base = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(number == base)
		return base;
	if(number < base)
	{
		close(base);
		return -2;
	}

	diff = number - base - 3;
	if(diff < 0)
	{
		close(base);
		return -3;
	}

	for(i = 0; i < diff; ++i)
	{
		t[i] = dup(base);
		if(t[i] < 0)
		{
			err = 1;
			diff = i - 1;
			break;
		}
	}

	if(err == 0)
		fd = dup(base);

	for(i = 0; i < diff; ++i)
		close(t[i]);

	close(base);
	return fd;
}

int kexecAllocSceUserEvent(size_t size)
{
	SceKernelEqueue queue;
	int r;

	r = sceKernelCreateEqueue(&queue, "kexec");
	if(r != 0)
		return -1;

	// this does not seem to need a valid fd ...
	// no idea if it works ...
	r = sceKernelAddUserEvent(queue, KExecChunkSizeCalulate(size));
	if(r != 0)
	{
		close(queue);
		return -2;
	}

	return queue;
}

int kexecAllocSceReadEventGlobal(size_t size)
{
	SceKernelEqueue queue;
	int r;

	r = sceKernelCreateEqueue(&queue, "kexec");
	if(r != 0)
		return -1;

	if(globalFileDescriptor == 0) // only one fd, thus size is used for all chunks
		globalFileDescriptor = kexecGenerateFileDescriptor(KExecChunkSizeCalulate(size));
	if(globalFileDescriptor < 0)
	{
		close(queue);
		return -2;
	}

	r = sceKernelAddReadEvent(queue, globalFileDescriptor, 0, NULL);
	if(r != 0)
	{
		close(queue);
		return -3;
	}

	return queue;
}

int kexecAllocKQueueGlobal(size_t size)
{
	int queue;
	struct kevent event;
	int r;

	queue = kqueue();
	if(queue < 0)
		return -1;

	if(globalFileDescriptor == 0)
		globalFileDescriptor = kexecGenerateFileDescriptor(KExecChunkSizeCalulate(size));
	if(globalFileDescriptor < 0)
	{
		close(queue);
		return -2;
	}

	EV_SET(&event, globalFileDescriptor, EVFILT_READ, EV_ADD, 0, 5, NULL);
	r = kevent(queue, &event, 1, NULL, 0, NULL);
	if(r < 0)
	{
		close(queue);
		return -3;
	}

	return queue;
}

int kexecAllocKQueue(size_t size)
{
	int queue;
	struct kevent event;
	int fd, r;

	queue = kqueue();
	if(queue < 0)
		return -1;

	fd = kexecGenerateFileDescriptor(KExecChunkSizeCalulate(size));
	if(fd < 0)
	{
		close(queue);
		return -2;
	}

	EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 5, NULL);
	r = kevent(queue, &event, 1, NULL, 0, NULL);
	if(r < 0)
	{
		close(queue);
		close(fd);
		return -3;
	}

	close(fd);
	return queue;
}

int kexecFree(int fd)
{
	if(fd < 0)
		return -1;

	return close(fd);
}

int kexecCloseGlobalIfNeeded()
{
	int r = 0;

	if(globalFileDescriptor == 0)
		return 0;

	if((kexecAlloc == kexecAllocKQueueGlobal) || (kexecAlloc == kexecAllocSceReadEventGlobal))
	{
		r = close(globalFileDescriptor);
		globalFileDescriptor = 0;
	}

	return r;
}
