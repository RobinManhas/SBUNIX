#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>

char* strcat(char *dest,const char *src);
char* strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, unsigned int n);
int strncmp(const char *s1, const char *s2, unsigned int n);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim , char **saveptr);
unsigned int strlen(const char *s);
char *strchr(const char *s, int c);
char* trimString(char* str);
void* memset(void* ptr, int val, unsigned int len);
long stoi(const char *s);
#endif