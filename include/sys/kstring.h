#ifndef _KSTRING_H
#define _KSTRING_H

int strcmp(const char *s1, const char *s2);

char* strcpy(char *dest, const char *src);
unsigned int strlen(const char *s);

char *strncpy(char *dest, const char *src, unsigned int n);
long stoi(const char *s);
void* memcpy( void* dest, const void* src, unsigned long count);

#endif