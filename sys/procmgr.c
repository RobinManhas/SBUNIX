//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/idt.h>


task_struct *t0,*t1,*t2;

void func1()
{
    kprintf("In thread #1\n");
    switch_to(t1,t2);
    kprintf("Returning from switchTo in thread #1\n");
    init_idt();
    init_irq();
    init_timer();
    init_keyboard();
    __asm__ ("sti");
    while(1);

}

void func2()
{
    kprintf("In thread #2\n");
    switch_to(t2,t1);
    kprintf("Returning from switchTo in thread #2\n");
    while(1);

}

void switch_to(task_struct *current, task_struct *next)
{
    __asm__ __volatile__("movq %%rsp, %0":"=r"(current->rsp));
    current->rsp+=8;
    __asm__ __volatile__("movq %0, %%rsp":: "r"(next->rsp));
    set_tss_rsp((void*)next->rsp);
    __asm__ __volatile__("ret");
}



void threadInit(){
    kprintf("In thread Init, size of task struct: %d\n", sizeof(task_struct));
    t0 = (task_struct*)kmalloc();
    t1 = (task_struct*)kmalloc();
    t2 = (task_struct*)kmalloc();

    t1->stack[499] = (uint64_t)&func1;
    t2->stack[499] = (uint64_t)&func2;

    t1->rsp = (uint64_t)&t1->stack[499];
    t2->rsp = (uint64_t)&t2->stack[499];

    t1->rip = (uint64_t)&func1;
    t2->rip = (uint64_t)&func2;

    switch_to(t0,t1);
}