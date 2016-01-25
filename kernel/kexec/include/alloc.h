#ifndef Alloc_H
#define Alloc_H

#include <stdlib.h>

// Some good to go stack sizes
#define KExecFileDescriptorMax 0x2800

// fd raise
int kexecGenerateFileDescriptor(int number);

// allocator
int kexecAllocKQueue(size_t size);
int kexecAllocKQueueGlobal(size_t size);
int kexecAllocSceUserEvent(size_t size); //   no idea
int kexecAllocSceReadEventGlobal(size_t size);

#define kexecAlloc kexecAllocKQueueGlobal

// free
int kexecFree(int fd);
int kexecCloseGlobalIfNeeded();

// Chunk size calculation control
#define KExecChunkSizeCalulate(size) (size - 0x800) / 8

#endif
