#ifndef _UTIL_H
#define _UTIL_H

void outb(unsigned short port, unsigned char val);
unsigned char inb (unsigned short port);
void outl(unsigned short port, unsigned int val);
unsigned int inl(unsigned short port);

#endif