#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

struct regs
{
    unsigned long r15,r14,r13,r12,r10,r9,r8,rbp,rdi,rsi,rdx,rbx,rax,rcx,r11;
    unsigned long int_no, err_code;
    unsigned long eip, cs, eflags, useresp, ss;
}__attribute__((packed));
void init_idt();
void init_irq();
void init_timer();
void init_keyboard();
void idt_set_gate(unsigned char num, long base, unsigned short sel, unsigned char flags);
void irq_install_handler(int irq, void (*handler)());


#endif // _IDT_H