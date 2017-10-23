#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

struct regs
{
    unsigned int rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */
};
void init_idt();
void init_irq();
void init_timer();
void init_keyboard();
void idt_set_gate(unsigned char num, long base, unsigned short sel, unsigned char flags);
void irq_install_handler(int irq, void (*handler)());


#endif // _IDT_H