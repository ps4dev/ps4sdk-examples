#ifndef Common_H
#define Common_H

#define _WANT_UCRED
#define _XOPEN_SOURCE 700
#define __BSD_VISIBLE 1

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <inttypes.h>

#include <sys/sysproto.h>
#include <sys/ucred.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/jail.h>
#include <sys/errno.h>
#include <sys/event.h>
#include <sys/syscall.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>

#ifdef __PS4__
#include <kernel.h>
#endif

#endif
