#include <sys/util.h>
#include <sys/defs.h>

void outb(unsigned short port, unsigned char val)
{
    __asm__ __volatile__ ( "outb %0,%1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
	* Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
	* The  outb  %al, %dx  encoding is the only option for all other cases.
	* %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}


unsigned char inb (unsigned short port){

    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outl(unsigned short port, unsigned int val)
{
    __asm__ __volatile__ ( "outl %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
	* Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
	* The  outb  %al, %dx  encoding is the only option for all other cases.
	* %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}


unsigned int inl(unsigned short port){

    unsigned int rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}