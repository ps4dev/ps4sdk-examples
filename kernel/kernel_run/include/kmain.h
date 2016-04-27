#ifndef KMainH
#define KMainH

#include <sys/sysent.h>

typedef int (*RunnableInt)();

int kmain(struct thread *td, void *uap);
int kmain2(struct thread *td, void *uap);
int kmain3(struct thread *td, void *uap);

#endif
