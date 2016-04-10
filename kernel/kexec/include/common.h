#ifndef Common_H
#define Common_H

#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/jail.h>
#include <sys/kthread.h>
#include <sys/sysproto.h>
#include <sys/ucred.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <sys/sockbuf.h>
#include <sys/socketvar.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#ifdef __PS4__
#include <kernel.h>
#include <ps4/resolve.h>
#endif

#endif
