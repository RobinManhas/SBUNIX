// References: http://www.osdever.net/bkerndev/Docs/idt.htm

#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/kprintf.h>


struct idt_entry
{
    unsigned short offset_1;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_2;
    unsigned int offset_3;
    unsigned int reserved;
} __attribute__((packed));

struct idt_ptr
{
    unsigned short limit;
    unsigned long base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void _idt_load(struct idt_ptr *idtp);


void idt_set_gate(unsigned char num, long base, unsigned short sel, unsigned char flags)
{

    idt[num].offset_1 = (base & 0xFFFFUL);
    idt[num].offset_2 = (base >> 16) & 0xFFFFUL;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
	idt[num].offset_3 = (base >> 32) & 0xFFFFFFFFUL;
    idt[num].reserved = 0x0;
}


void init_idt()
{
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = (long)idt;
    _idt_load(&idtp);
}