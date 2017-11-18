// References: http://www.osdever.net/bkerndev/Docs/isrs.htm

#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/util.h>

#define PRES 		0x80
#define DPL_0 		0x00
#define DPL_1 		0x20
#define DPL_2 		0x40
#define DPL_3 		0x60
#define S 		    0x00
#define INTR_GATE 	0x0E

void _irq0();
void _irq1();
void isr0();
void syscall();
extern void syscall_handler();

void *irqs[16] =
        {
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0
        };

void irq_install_handler(int irq, void (*handler)())
{
    irqs[irq] = handler;

}


void irq_uninstall_handler(int irq)
{
    irqs[irq] = 0;
}




void irq_remap()
{

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);


}


void init_irq()
{
    unsigned char ring0Attr = PRES | DPL_0 | S | INTR_GATE;
    unsigned char ring3Attr = PRES | DPL_3 | S | INTR_GATE;
    irq_remap();
    idt_set_gate(32, (long)_irq0, 0x08, ring0Attr);
    idt_set_gate(33, (long)_irq1, 0x08, ring0Attr);
    idt_set_gate(0, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(1, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(2, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(3, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(4, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(5, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(6, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(7, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(8, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(9, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(10, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(11, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(12, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(13, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(14, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(15, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(16, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(17, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(18, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(19, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(20, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(21, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(22, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(23, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(24, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(25, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(26, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(26, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(28, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(27, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(29, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(30, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(31, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(128, (long)syscall, 0x08, ring3Attr);

    //outb(0x21,0xFD);   // to disable irq lines : 0xFD=11111101 enable only keyboard(0 to enable)
}


void _irq_handler(struct regs* reg)
{
    if(reg->int_no==128){
        kprintf("syscall interrupt received\n");
        //syscall_handler(reg);
    }else {
        long num = (reg->int_no) - 32;

        void (*handler)();

        handler = irqs[num];
        if (handler) {
            handler();
        }
    }


//    if (num >= 40)
//    {
//        outb(0xA0, 0x20);
//    }


    outb(0x20, 0x20);//for irq lines

}
