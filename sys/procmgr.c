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
#include <sys/util.h>

uint16_t processID = 0; // to keep track of the allocated process ID's to task struct
extern uint64_t kernel_rsp;
extern task_struct *kernel_idle_task; // this store the idle task struct. this task is run when no other active task are available.
task_struct* gReadyList = NULL;
task_struct* gBlockedList = NULL;
task_struct* gZombieList = NULL;

task_struct *current=NULL, *prev=NULL;

task_struct* tasks_list[100];

task_struct* getCurrentTask(){
    return current;
}

/* init function */
void runner(){
    while(1) {
        kprintf("inside idle\n");
        schedule();
    }
}

void userFunc(){
    uint64_t ret =0;
    uint64_t syscall = 1;
    uint64_t arg3 = 1;
    //__asm__ __volatile__ ("movq %1,%%rax;syscall" : "=r" (ret) : "0" (syscall):"memory");
    int c = 99;
    uint64_t arg2=(uint64_t )&c;
    uint64_t arg1 = 1;
    __asm__ __volatile__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi; movq %4,%%rdx;syscall" : "=r" (ret):"0"(syscall), "g"(arg1), "g"(arg2) ,"g"(arg3) :"memory" );

    while(1);
}

void func1()
{
    kprintf("Thread 1: Entry, task ID: %d\n",current->pid);
    schedule();
    kprintf("Thread 1: Returning from switch first time\n");
    schedule();
    kprintf("Thread 1: Returning from switch second time\n");
    schedule();
    kprintf("Thread 1: Returning from switch third time\n");
//    init_idt();
//    init_irq();
//    kprintf("Thread 1: init IDT and IRQ success\n");
    //createUserProcess();

//    init_timer();
//    init_keyboard();
//    __asm__ ("sti");
    while(1);

}

void func2()
{
    kprintf("Thread 2: Entry, task ID: %d\n",current->pid);
    schedule();
    kprintf("Thread 2: Returning from switch first time\n");
    schedule();
    kprintf("Thread 2: Returning from switch second time\n");
    schedule();
    kprintf("Thread 2: Returning from switch third time\n");
    //schedule();
    while(1);

}

void addTaskToReady(task_struct *readyTask){
    if(readyTask == NULL){
        kprintf("Error: invalid task in add to ready, returning\n");
        return;
    }

    readyTask->next = NULL;
    if(gReadyList == NULL)
    {
        gReadyList = readyTask;
    }
    else
    {
        //RM: add to end of ready list
        task_struct *iter = gReadyList;
        if(iter == readyTask){ // checks being added as a process got pushed to list multiple times
            kprintf("Error: ready task already exists, returning\n");
            return;
        }
        while(iter->next != NULL){
            if(iter == readyTask){
                kprintf("Error: ready task already exists, returning\n");
                return;
            }
            iter = iter->next;
        }

        iter->next = readyTask;
    }
}

void addTaskToBlocked(task_struct *blockedTask){
    if(blockedTask == NULL){
        kprintf("Error: invalid task in add to blocked, returning\n");
        return;
    }

    blockedTask->next = NULL;
    if(gBlockedList == NULL)
    {
        gBlockedList = blockedTask;
    }
    else
    {
        //RM: add to end of blocked list
        task_struct *iter = gBlockedList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = blockedTask;
    }
}

void addTaskToZombie(task_struct *zombieTask){
    if(zombieTask == NULL){
        kprintf("Error: invalid task in add to zombie, returning\n");
        return;
    }

    zombieTask->next = NULL;
    if(gZombieList == NULL)
    {
        gZombieList = zombieTask;
    }
    else
    {
        //RM: add to end of zombie list
        task_struct *iter = gZombieList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = zombieTask;
    }
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

    if(next->init == 1){
        next->init = 0;
    }
    else{
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
}

void schedule()
{
    if(current != NULL /*gReadyList != NULL*/){
        prev = current;
        current = gReadyList;
        if(current == NULL)
            current = kernel_idle_task; // TODO: currently does not switch properly
        else
            gReadyList = gReadyList->next;

        // add prev task switched to end of ready list
        switch(prev->state)
        {
            case TASK_STATE_RUNNING:
            {
                addTaskToReady(prev);
                break;
            }
            case TASK_STATE_BLOCKED:
            {
                addTaskToBlocked(prev);
                break;
            }
            case TASK_STATE_ZOMBIE:
            {
                addTaskToZombie(prev);
                break;
            }
            case TASK_STATE_IDLE:
            {
                // don't add idle task to any queue.
                break;
            }
            default:
            {
                kprintf("unhandled task state in scheduler\n");
                break;
            }
        }

        if(current->type == TASK_KERNEL)
            switch_to(prev,current);
        else if(current->type == TASK_USER)
            switch_to_user_mode(prev,current);
    }

}

uint16_t getFreePID()
{
    return processID++; // 0 will be our initial kernel task
}

task_struct* getFreeTask()
{
    task_struct *task = (task_struct*)kmalloc();
    task->pid = getFreePID();
    tasks_list[task->pid] = task;
    return task;
}

void createKernelInitProcess(task_struct *ktask){
    ktask->stack = kmalloc();
    ktask->init = 1;
    ktask->stack[510] = (uint64_t)runner;
    ktask->rsp = (uint64_t)&ktask->stack[510];
    ktask->cr3 = (uint64_t)getKernelPML4();
    ktask->rip = (uint64_t) &runner;
    ktask->type = TASK_KERNEL;
    ktask->state = TASK_STATE_IDLE;
    current = ktask;

}

void createKernelTask(task_struct *task, void (*func)(void)){

    task->stack = kmalloc();
    task->init = 1;
    task->stack[510] = (uint64_t)func;
    task->rsp = (uint64_t)&task->stack[510];
    task->rip = (uint64_t)&func1;
    task->cr3 = (uint64_t)getKernelPML4();
    task->type = TASK_KERNEL;
    task->state = TASK_STATE_RUNNING;
    addTaskToReady(task);
}

void createUserProcess(task_struct *user_task){
    uint64_t userbase = 0x88880000000UL;
    user_task->type = TASK_USER;
    user_task->state = TASK_STATE_RUNNING;
    user_task->cr3 = (uint64_t)kmalloc();
    user_task->stack = kmalloc();
    user_task->rip = (uint64_t)&userFunc;
    user_task->rsp = (uint64_t)&user_task->stack[499];
    user_task->kernInitRSP = (uint64_t)&user_task->stack[499];
    user_task->fd[0]=create_terminal_IN();
    FD* filedec = create_terminal_OUT();
    user_task->fd[1]= filedec;
    user_task->fd[2]= filedec;

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
    map_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),&userPtr);
    userbase+=0x1000;

    // map stack
    map_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),&userPtr);
    user_task->stack = (uint64_t*)userbase;
    userbase+=0x1000;

    // map rsp
    user_task->rsp = (uint64_t)&user_task->stack[499];

    // map user page rip
    map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr);
    user_task->rip = userbase | (((uint64_t)&userFunc) & ~ADDRESS_SCHEME); //RM: pop address offset

    // map kernel
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
    kprintf("User process ready, kernFunc: %x, ring3Func: %x\n",&userFunc,user_task->rip);
    addTaskToReady(user_task);
}

void switch_to_user_mode(task_struct *oldTask, task_struct *user_task)
{
    __asm__ volatile("cli");
//    __asm__ __volatile__("pushq %rax");
//    __asm__ __volatile__("pushq %rbx");
//    __asm__ __volatile__("pushq %rcx");
//    __asm__ __volatile__("pushq %rdx");
//    __asm__ __volatile__("pushq %rdi");
//    __asm__ __volatile__("pushq %rsi");
//    __asm__ __volatile__("pushq %rbp");
//    __asm__ __volatile__("pushq %r8");
//    __asm__ __volatile__("pushq %r9");
//    __asm__ __volatile__("pushq %r10");
//    __asm__ __volatile__("pushq %r11");
//    __asm__ __volatile__("pushq %r12");
//
//    __asm__ __volatile__("movq %%rsp, %0":"=r"(oldTask->rsp));

    set_tss_rsp((void*)(user_task->kernInitRSP - 16));
    kernel_rsp = (user_task->kernInitRSP - 16);

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

    mm_struct* mm = (is_user_task ==1)?(mm_struct*)umalloc_size(sizeof(mm_struct)):(mm_struct*)kmalloc_size(sizeof(mm_struct));


    task->mm = mm;
    task->cr3 = get_new_cr3(is_user_task); //to be modified

    task->rsp = (uint64_t)&task->stack[499];

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)task->cr3;
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
    return task;

}

void killActiveProcess(){
    current->state = TASK_STATE_ZOMBIE;
    // TODO: If any parent, reduce parent's child count. If child, find in ready or blocked queue and move to zombie
    schedule();
}