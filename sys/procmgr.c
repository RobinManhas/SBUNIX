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
#include <sys/kstring.h>

task_struct *t0,*t1,*t2,*user_task;
void userFunc(){
    __asm__ __volatile__("int $0x80");
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
    init_idt();
    init_irq();
    kprintf("Thread 1: init IDT and IRQ success\n");
    createUserProcess();

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
    t1->stack = kmalloc();
    t2->stack = kmalloc();
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
    uint64_t userbase = 0x88880000000UL;
    user_task = (task_struct*)kmalloc();
    user_task->cr3 = (uint64_t)kmalloc();
    user_task->stack = kmalloc();
    user_task->rip = (uint64_t)&userFunc;
    user_task->rsp = (uint64_t)&user_task->stack[499];

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)user_task->cr3;
    userPtr[510] = returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1);
    userPtr[510] |= (PTE_U_W_P);

    uint64_t userPage = (uint64_t)kmalloc();
    uint64_t kernPage = (((uint64_t)&userFunc) & ADDRESS_SCHEME);
    memcpy((void*)userPage,(void*)kernPage ,PAGE_SIZE);
    //NOTE: kernPage can be replace by the following to map function to start of new physical page
    // (void*)(kernPage | (((uint64_t)&userFunc) & ~ADDRESS_SCHEME))

    /* RM: change everything in accordance to user process at this point    *
     * map pml4                                                             */
    //map_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),&userPtr);
    userbase+=0x1000;

    // map stack
    //map_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),&userPtr);
    user_task->stack = (uint64_t*)userbase;
    userbase+=0x1000;

    // map rsp
    user_task->rsp = (uint64_t)&user_task->stack[499];

    // map user page rip
    //map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr);
    user_task->rip = userbase | (((uint64_t)&userFunc) & ~ADDRESS_SCHEME); //RM: pop address offset

    // map kernel
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);

    kprintf("before switch, kernFunc: %x, ring3Func: %x\n",&userFunc,user_task->rip);
    switch_to_user_mode(user_task);
}

void switch_to_user_mode(task_struct *user_task)
{
    set_tss_rsp((void*)t1->rsp);
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

task_struct* allocate_task(int is_user_task){

    uint64_t task_struct_size = sizeof(task_struct);
    task_struct* task=(is_user_task ==1)?(task_struct*)umalloc_size(task_struct_size):(task_struct*)kmalloc_size(
            task_struct_size);

    mm_struct* mm = (is_user_task ==1)?(mm_struct*)umalloc():(mm_struct*)kmalloc();


    task->mm = mm;
    task->cr3 = get_new_cr3(is_user_task); //to be modified

    task->rip = (uint64_t)&userFunc;
    task->rsp = (uint64_t)&user_task->stack[499];

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)task->cr3;
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
    return task;

}


