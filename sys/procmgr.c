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
    kprintf("Thread 1: Entry\n");
    init_switch_to(t1, t2);
    kprintf("Thread 1: Returning from switch first time\n");
    switch_to(t1, t2);
    kprintf("Thread 1: Returning from switch second time\n");
    switch_to(t1, t2);
    kprintf("Thread 1: Returning from switch third time\n");
    init_idt();
    init_irq();
    init_timer();
    init_keyboard();
    __asm__ ("sti");
    while(1);

}

void func2()
{
    kprintf("Thread 2: Entry\n");
    switch_to(t2, t1);
    kprintf("Thread 2: Returning from switch first time\n");
    switch_to(t2, t1);
    kprintf("Thread 2: Returning from switch second time\n");
    switch_to(t2, t1);
    kprintf("Thread 2: Returning from switch third time\n");
    while(1);

}

void init_switch_to(task_struct *current, task_struct *next)
{
    __asm__ __volatile__("pushq %rax");
    __asm__ __volatile__("pushq %rbx");
    __asm__ __volatile__("pushq %rcx");
    __asm__ __volatile__("pushq %rdx");
    __asm__ __volatile__("pushq %rdi");
    __asm__ __volatile__("pushq %rsi");
    __asm__ __volatile__("pushq %rbp");
    __asm__ __volatile__("pushq %r8");
    __asm__ __volatile__("pushq %r9");
    __asm__ __volatile__("pushq %r10");
    __asm__ __volatile__("pushq %r11");
    __asm__ __volatile__("pushq %r12");

    __asm__ __volatile__("movq %%rsp, %0":"=r"(current->rsp));
    __asm__ __volatile__("movq %0, %%rsp":: "r"(next->rsp));

}

void switch_to(task_struct *current, task_struct *next)
{
    __asm__ __volatile__("pushq %rax");
    __asm__ __volatile__("pushq %rbx");
    __asm__ __volatile__("pushq %rcx");
    __asm__ __volatile__("pushq %rdx");
    __asm__ __volatile__("pushq %rdi");
    __asm__ __volatile__("pushq %rsi");
    __asm__ __volatile__("pushq %rbp");
    __asm__ __volatile__("pushq %r8");
    __asm__ __volatile__("pushq %r9");
    __asm__ __volatile__("pushq %r10");
    __asm__ __volatile__("pushq %r11");
    __asm__ __volatile__("pushq %r12");

    __asm__ __volatile__("movq %%rsp, %0":"=r"(current->rsp));
    __asm__ __volatile__("movq %0, %%rsp":: "r"(next->rsp));

    __asm__ __volatile__("popq %r12");
    __asm__ __volatile__("popq %r11");
    __asm__ __volatile__("popq %r10");
    __asm__ __volatile__("popq %r9");
    __asm__ __volatile__("popq %r8");
    __asm__ __volatile__("popq %rbp");
    __asm__ __volatile__("popq %rsi");
    __asm__ __volatile__("popq %rdi");
    __asm__ __volatile__("popq %rdx");
    __asm__ __volatile__("popq %rcx");
    __asm__ __volatile__("popq %rbx");
    __asm__ __volatile__("popq %rax");

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

    init_switch_to(t0, t1);
}