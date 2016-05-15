#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1
#define _KERNEL
#define _WANT_UCRED
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/sysproto.h>

#include <machine/specialreg.h>

#undef offsetof
#include <ps4/kernel.h>
#include <ps4/kern.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/filedesc.h>
#include <sys/filio.h>
#include <sys/fcntl.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/poll.h>
#include <sys/resourcevar.h>
#include <sys/selinfo.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/condvar.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif

extern int kern_writev(struct thread *td, int fd, struct uio *auio);

int kern_sys_write(struct thread *td, struct write_args *uap)
{
	struct uio auio;
	struct iovec aiov;
	int error;

	if (uap->nbyte > INT_MAX)
		return (EINVAL);

	aiov.iov_base = (void *)(uintptr_t)uap->buf;
	aiov.iov_len = uap->nbyte;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = uap->nbyte;
	auio.uio_segflg = UIO_SYSSPACE;
	error = kern_writev(td, uap->fd, &auio);

	return(error);
}

int main(int argc, char **argv)
{
	void *memory = ps4KernMemoryMalloc(1234);
	struct write_args wargs; //Die monster! You don't belong in this world!

	sprintf(memory, "sys_write: %p, memory: %p\n", (void *)sys_write, (void *)memory);
	wargs.fd = 1;
	wargs.buf = memory;
	wargs.nbyte = strnlen(memory, 1234);
	int r = kern_sys_write(ps4KernThreadCurrent(), &wargs);
	ps4KernMemoryFree(memory);

	return r;
}
