#include <sys/util.h>
#include <sys/defs.h>
#include <sys/common.h>

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

uint64_t pow(uint64_t x, int n){
    if (n == 0)
        return 1;
    return x * pow(x, n-1);
}

uint64_t octalToDecimal(uint64_t octal){
    uint64_t decimal = 0;
    uint64_t i=0;
    while(octal!=0){
        decimal = decimal + (octal % 10) * pow(8,i++);
        octal = octal/10;
    }
    return decimal;
}

uint64_t getRSP() {
    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));
    return ret;
}