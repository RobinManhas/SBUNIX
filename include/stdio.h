#ifndef _STDIO_H
#define _STDIO_H

#include <unistd.h>

static const int EOF = -1;

#define MAX_READ_BYTES 100
int putchar(int c);
void putn(long n);
int puts(const char *s);
int printf(const char *format, ...);
int putVal(const char *s);
void clearScr();
char *gets(char *s);
int getch();



int fileOpen(void *filename, unsigned int flag);
int filegets(char *str , int size,int fd);
int getdir(void* buf, int size);
int chdir(const char *buf);
int execve(char *filename, char* args[], char* envs[]);

void* sys_brk(size_t size);
int sys_fstat(int fd ,void* file);
int sys_lseek(int fd, int offset, int position);

int dup2(int fd, int newfd);
int dup(int oldfd);
int sys_ps( void *buf, int count);

#endif
