// References: http://www.osdever.net/bkerndev/Docs/isrs.htm

#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/util.h>
#include <sys/vmm.h>
#include <sys/mm.h>
#include <sys/procmgr.h>
#include <sys/pmm.h>
#include <sys/kstring.h>
#include <sys/common.h>


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
void isr14();
void syscall();
void isr13();

extern void syscall_handler();
void handle_page_fault(struct regs* reg);

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
    idt_set_gate(32, (long)_irq0, 0x08, ring3Attr);
    idt_set_gate(33, (long)_irq1, 0x08, ring3Attr);
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
    idt_set_gate(13, (long)isr13, 0x08, ring0Attr);
    idt_set_gate(14, (long)isr14, 0x08, ring0Attr);
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

    outb(0x21,0xFC);   // to disable irq lines : 0xFD=11111101 enable only keyboard(0 to enable)
}


void _irq_handler(struct regs* reg)
{
    if(reg->int_no==128){
#ifdef DEBUG_LOGS_ENABLE
        kprintf("syscall interrupt received, %d\n",reg->rax);
#endif
    }
    else if(reg->int_no==14){
        handle_page_fault(reg);
    }else if(reg->int_no==13){
#ifdef ERROR_LOGS_ENABLE
        kprintf("got interrupt no %d\n",reg->int_no);
        kprintf("got error no %d\n",reg->err_code);
#endif
        uint64_t faulty_addr;
        __asm__ __volatile__ ("movq %%cr2, %0;" : "=r"(faulty_addr));
#ifdef ERROR_LOGS_ENABLE
        kprintf("error:%x\n",faulty_addr);
#endif
        __asm__ volatile("hlt;":::);
    }
    else {
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

void handle_page_fault(struct regs* reg){
    task_struct* current_task = getCurrentTask();
    uint64_t faulty_addr;
    __asm__ __volatile__ ("movq %%cr2, %0;" : "=r"(faulty_addr));
#ifdef ERROR_LOGS_ENABLE
    kprintf("inside page fault,error addr:%x\n",faulty_addr);
#endif
    uint64_t err_code = reg->err_code;
    uint64_t new_page,new_vir;
    uint64_t * pml4_pointer = (uint64_t*)current_task->cr3;


    //err_code 0bit-> if set; then page is present
    if(err_code & 0x1){
        //get physical address
        uint64_t phy_addr = getPTEntry(faulty_addr);

        //not writable and cow set
        if(!(phy_addr & PTE_W) && (phy_addr & PTE_COW) ){
            //check if shared
            Page* page = get_page(phy_addr);
            if(page != NULL && page->sRefCount >1){
                new_page = allocatePage();
                new_vir = current_task->mm->v_addr_pointer;
                current_task->mm->v_addr_pointer += 0x1000;
                map_user_virt_phys_addr(new_vir,new_page,&pml4_pointer,1);



                uint64_t tmp_vir = current_task->mm->v_addr_pointer;
                current_task->mm->v_addr_pointer += 0x1000;
                map_user_virt_phys_addr(tmp_vir,phy_addr,&pml4_pointer,1);

                //copy contents from old page to new page
                kmemcpy((uint64_t *)new_vir,(uint64_t *)tmp_vir,PAGE_SIZE);

                phy_addr = new_page|PTE_U_W_P;
                page->sRefCount--;

            }else{
                //unset cow and set write bit
                phy_addr = phy_addr | PTE_W;
                phy_addr = phy_addr &(~PTE_COW);
            }

            setPTEntry(faulty_addr,phy_addr);
            __asm__ __volatile__ ("invlpg (%0)" ::"r" (faulty_addr) : "memory");
        }else{
#ifdef DEBUG_LOGS_ENABLE
            kprintf("reason for page fault is unknown \n");
#endif
            __asm__ __volatile__ ("hlt");
            killTask(current_task);
        }

    }else {
        //page not present
        if(current_task->mm!=NULL) {
            vm_area_struct *vma = find_vma(current_task->mm, faulty_addr);
            if (vma == NULL) {
#ifdef ERROR_LOGS_ENABLE
                kprintf("ERROR: page fault address is out of bound");
#endif
               // __asm__ __volatile__ ("hlt");
                killTask(current_task);
            }
            //allocate all pages to vma if file is present
            if (vma->file != NULL)
                allocate_pages_to_vma(vma, &pml4_pointer);
            else
                allocate_single_page(current_task, faulty_addr);
        }else{
#ifdef ERROR_LOGS_ENABLE
            kprintf("ERROR: page fault\n");
#endif
           // __asm__ __volatile__ ("hlt");
            killTask(current_task);
        }


    }
    return;


}





