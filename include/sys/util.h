#ifndef _UTIL_H
#define _UTIL_H

#include <sys/defs.h>

void outb(unsigned short port, unsigned char val);
unsigned char inb (unsigned short port);
void outl(unsigned short port, unsigned int val);
unsigned int inl(unsigned short port);
uint64_t pow(uint64_t x, int n);
uint64_t octalToDecimal(uint64_t octal);
uint64_t getRSP();
#endif