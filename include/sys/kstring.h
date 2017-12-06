#ifndef _KSTRING_H
#define _KSTRING_H

int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, unsigned int n);
char* kstrcpy(char *dest, const char *src);
unsigned int kstrlen(const char *s);

char *kstrncpy(char *dest, const char *src, unsigned int n);
long kstoi(const char *s);
void* kmemcpy( void* dest, const void* src, unsigned long count);
char* kstrcat(char *dest,const char *src);
int ktostring(char buf[], int num);

#endif