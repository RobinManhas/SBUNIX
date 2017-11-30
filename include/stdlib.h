#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

int main();
void exit(int status);

void *malloc(size_t size);
void free(void *ptr);

int setenv(const char *envname, const char *envval, int overwrite);
char *getenv(const char *name);

#endif
