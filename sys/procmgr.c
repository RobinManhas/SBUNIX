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

task_struct *t0,*t1,*t2,*user_task;
void userFunc(){
    kprintf("User function entry\n");
    while(1);
}

void func1()
{
    kprintf("Thread 1: Entry\n");
//    init_switch_to(t1, t2);
//    kprintf("Thread 1: Returning from switch first time\n");
//    switch_to(t1, t2);
//    kprintf("Thread 1: Returning from switch second time\n");
//    switch_to(t1, t2);
//    kprintf("Thread 1: Returning from switch third time\n");
    createUserProcess();
//    init_idt();
//    init_irq();
//    init_timer();
//    init_keyboard();
//    __asm__ ("sti");
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

    t1->cr3 = (uint64_t)getKernelPML4();
    t2->cr3 = (uint64_t)getKernelPML4();

    init_switch_to(t0, t1);
}

void createUserProcess(){
    user_task = (task_struct*)kmalloc();
    user_task->cr3 = (uint64_t)kmalloc();
    user_task->rip = (uint64_t)&userFunc;

    user_task->rsp = (uint64_t)&user_task->stack[499];
    kprintf("rip: %x\n",user_task->rip);

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)user_task->cr3;
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    kprintf("upt: %x - %x, ua: %x, ka: %x\n",userPtr,user_task->cr3,userPtr[511], kernPtr[511]);
    switch_to_user_mode(user_task);
}

void switch_to_user_mode(task_struct *user_task)
{
    set_tss_rsp((void*)user_task->rsp);
    __asm__ volatile("cli");
    setCR3((uint64_t*)user_task->cr3);
    __asm__ volatile("mov $0x23, %%ax"::);
    __asm__ volatile("mov %%ax, %%ds"::);
    __asm__ volatile("mov %%ax, %%es"::);
    __asm__ volatile("mov %%ax, %%fs"::);
    __asm__ volatile("mov %%ax, %%gs"::);

    __asm__ volatile("movq %0, %%rax"::"r"(user_task->rsp));
    __asm__ volatile("pushq $0x23");
    __asm__ volatile("pushq %rax");
    __asm__ volatile("pushfq");
    __asm__ volatile("pushq $0x2B");
    __asm__ volatile("pushq %0"::"r"(user_task->rip));
    __asm__ volatile("iretq");
}