#ifndef Util_H
#define Util_H

#include <stdlib.h>
#include <stdio.h>

int printBytes(unsigned char *str, size_t size);
int printPrintableBytes(unsigned char *str, size_t size);

int stringBytes(unsigned char *out, unsigned char *str, size_t size);
int stringPrintableBytes(unsigned char *out, unsigned char *str, size_t size);

int utilServerCreate(int port, int backlog, int try, unsigned int sec);
int utilSingleAcceptServer(int port);

#endif
