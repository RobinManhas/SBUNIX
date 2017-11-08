//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/procmgr.h>

task_struct *current,*next;
int f=0;
void func1()
{
    while(1)
    {
        if(f<10)
        {
            f += 1;
            kprintf("I'm thread #1\n");
            task_struct *temp = current;
            current = next;
            next = temp;
            switch_to();
        }

    }
}
int flag = 0;
void func2()
{
    while(1)
    {
        if(flag<10)
        {
            flag += 1;
            kprintf("I'm thread #2\n");
            task_struct *temp = current;
            current = next;
            next = temp;
            switch_to();
        }

    }
}

void switch_to()
{
    void *updatedRSP = next->stack;
    __asm volatile("movq %%rsp, %0":: "r"(&(current->rsp)));
    __asm volatile("movq %0, %%rsp":: "r"(next->rsp));
    __asm volatile("movq %0, %%rax":: "r"(next->rip));
    __asm volatile("pushq %%rax"::);
    __asm volatile("movq %%rsp, %0":: "r"(&(updatedRSP)));
    set_tss_rsp(updatedRSP);
    __asm volatile("ret");
}

void threadInit(){
    kprintf("size of task struct: %d, uint64: %x\n", sizeof(task_struct), sizeof(uint64_t));
    current = (task_struct*)kmalloc();
    next = (task_struct*)kmalloc();

    current->rsp = (uint64_t)&current->stack[499];
    next->rsp = (uint64_t)&next->stack[499];
    kprintf("current stack: %x, add: %x,rsp: %x\n", current->stack, &current->stack,current->rsp);
//current stack: 0xffffffff802d2018, add: 0xffffffff802d2018,rsp: 0xffffffff802d2fb0
    current->rip = (uint64_t)&func1;
    next->rip = (uint64_t)&func2;
    switch_to();
}