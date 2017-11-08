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

task_struct *t1,*t2;
int f1flag=0;
void func1()
{
    while(1)
    {
        if(f1flag<10)
        {
            f1flag += 1;
            kprintf("I'm thread #1\n");
            switch_to(t1,t2);
        }

    }
}
int f2flag = 0;
void func2()
{
    while(1)
    {
        if(f2flag<10)
        {
            f2flag += 1;
            kprintf("I'm thread #2\n");
            switch_to(t2,t1);
        }

    }
}

void switch_to(task_struct *current, task_struct *next)
{
    void *updatedRSP = (void*)next->rsp;
    __asm volatile("movq %%rsp, %0":: "r"(&(current->rsp)));
    __asm volatile("movq %0, %%rsp":: "r"(next->rsp));
    __asm volatile("movq %0, %%rax":: "r"(next->rip));
    __asm volatile("pushq %%rax"::);
    __asm volatile("movq %%rsp, %0":: "r"(&(updatedRSP)));
    set_tss_rsp(updatedRSP);
    __asm volatile("ret");
}

void threadInit(){
    kprintf("In thread Init, size of task struct: %d\n", sizeof(task_struct));
    t1 = (task_struct*)kmalloc();
    t2 = (task_struct*)kmalloc();

    t1->rsp = (uint64_t)&t1->stack[499];
    t2->rsp = (uint64_t)&t2->stack[499];

    t1->rip = (uint64_t)&func1;
    t2->rip = (uint64_t)&func2;
    switch_to(t1,t2);
}