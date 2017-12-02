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
#include <sys/mm.h>
#include <sys/terminal.h>

uint16_t processID = 0; // to keep track of the allocated process ID's to task struct
extern uint64_t kernel_rsp;
extern task_struct *kernel_idle_task; // this store the idle task struct. this task is run when no other active task are available.
task_struct* gReadyList = NULL;
task_struct* gBlockedList = NULL;
task_struct* gZombieList = NULL;

task_struct *currentTask=NULL, *prevTask=NULL;

task_struct* tasks_list[100];

task_struct* getCurrentTask(){
    return currentTask;
}
int counter = 0;
/* init function */
void runner(){
    while(1) {
//        kprintf("inside kernel idle runner: %d\n",counter++);
//        if(counter == 10000) // wrap around
//            counter = 0;
        schedule();
    }
}

void userFunc(){
//    uint64_t ret =0;
//    uint64_t syscall = 1;
//    uint64_t arg3 = 5;
//    //__asm__ __volatile__ ("movq %1,%%rax;syscall" : "=r" (ret) : "0" (syscall):"memory");
//    char buff[]="hello";
//    uint64_t arg2=(uint64_t )buff;
//    uint64_t arg1 = 1;
//    __asm__ __volatile__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi; movq %4,%%rdx;syscall" : "=r" (ret):"0"(syscall), "g"(arg1), "g"(arg2) ,"g"(arg3) :"memory" );
//    //schedule();
//    while(1);
}
void createUserProcess_temp(task_struct *user_task){
    uint64_t userbase = VIRBASE;
    user_task->type = TASK_USER;
    user_task->no_of_children = 0;
    user_task->next = NULL;
    user_task->nextChild = NULL;

    user_task->fd[0]=create_terminal_IN();
    FD* filedec = create_terminal_OUT();
    user_task->fd[1]= filedec;
    user_task->fd[2]= filedec;

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)user_task->cr3;
    userPtr[510] = returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1);
    userPtr[510] |= (PTE_U_W_P);

    map_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),&userPtr);
    userbase+=0x1000;

    // map stack
    map_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),&userPtr);
    user_task->stack = (uint64_t*)userbase;
    userbase+=0x1000;

    uint64_t userPage = (uint64_t)kmalloc();
    map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr);
    userbase+=0x1000;

    user_task->mm = (mm_struct*)userPage;
    user_task->mm->v_addr_pointer = userbase;
    user_task->mm->vma_cache = NULL;

    // map kernel
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
}
void func1()
{
//    kprintf("Thread 1: Entry, task ID: %d\n",current->pid);
//    schedule();
//    kprintf("Thread 1: Returning from switch first time\n");
//    schedule();
//    kprintf("Thread 1: Returning from switch second time\n");
//    schedule();
//    kprintf("Thread 1: Returning from switch third time\n");
//    init_idt();
//    init_irq();
//    kprintf("Thread 1: init IDT and IRQ success\n");
    //createUserProcess();

//    init_timer();
//    init_keyboard();
//    __asm__ ("sti");
    createUserProcess_temp(getCurrentTask());
    char * argv[]={"/bin/sbush","/temp", NULL};
    char * envp[]={"PATH=/bin:", "HOME=/root", "USER=root", NULL};
    load_elf_binary_by_name(getCurrentTask(),"bin/sbush",argv,envp);
    switch_to_user_mode(NULL,getCurrentTask());
    while(1);

}

void func2()
{
    kprintf("Thread 2: Entry, task ID: %d\n",currentTask->pid);
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
            kprintf("Error: ready task already exists : %d, returning\n",readyTask->pid);
            return;
        }
        while(iter->next != NULL){
            if(iter == readyTask){
                kprintf("Error: ready task already exists : %d, returning\n",readyTask->pid);
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

    if(next->state == TASK_STATE_KERNEL_RUNNER)
    { // just put runner function into rsp and return, idle task can start from function beginning, hence do not need saving of registers
        next->stack[510] = (uint64_t)runner;
        next->rsp = (uint64_t)&next->stack[510];
        __asm__ __volatile__("movq %0, %%rsp":: "r"(next->rsp));
    }
    else{
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

}

void schedule()
{
    if(currentTask != NULL /*gReadyList != NULL*/){
        prevTask = currentTask;
        currentTask = gReadyList;
        if(currentTask == NULL){
            if(prevTask->state == TASK_STATE_KERNEL_RUNNER)
            {
                currentTask = prevTask;
                switch_to(prevTask,currentTask);
            }
            else if(prevTask->state == TASK_STATE_RUNNING){
                currentTask = prevTask;
                return;
            }
            else{ // cannot run previous task too, run idle task
                currentTask = kernel_idle_task;
                switch_to(prevTask,currentTask);
            }
        }
        else
            gReadyList = gReadyList->next;

        // add prevTask task switched to end of ready list
        switch(prevTask->state)
        {
            case TASK_STATE_RUNNING:
            {
                addTaskToReady(prevTask);
                break;
            }
            case TASK_STATE_BLOCKED:
            {
                addTaskToBlocked(prevTask);
                break;
            }
            case TASK_STATE_ZOMBIE:
            {
                addTaskToZombie(prevTask);
                break;
            }
            case TASK_STATE_IDLE:
            case TASK_STATE_KILLED:
            case TASK_STATE_KERNEL_RUNNER:
            {
                // don't add idle task or killed task to any queue.
                break;
            }
            default:
            {
                kprintf("unhandled task state in scheduler\n");
                break;
            }
        }

//        if(current->type == TASK_KERNEL)
//            switch_to(prev,current);
//        else if(current->type == TASK_USER)
//            switch_to_user_mode(prev,current);
        switch_to(prevTask,currentTask);
        set_tss_rsp((uint64_t *)currentTask->rsp);
        kernel_rsp = currentTask->rsp;

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
   // kprintf("assigned task add: %x, pid: %x\n",task,task->pid);
    tasks_list[task->pid] = task;
    return task;
}

void createKernelInitProcess(task_struct *ktask){
    ktask->stack = kmalloc();
    ktask->init = 1;
    ktask->stack[510] = (uint64_t)runner;
    ktask->rsp = (uint64_t)&ktask->stack[510];
    ktask->cr3 = (uint64_t)getKernelPML4();
    ktask->user_rip = (uint64_t) &runner;
    ktask->type = TASK_KERNEL;
    ktask->state = TASK_STATE_KERNEL_RUNNER;
    ktask->no_of_children = 0;
    ktask->next = NULL;
    ktask->nextChild = NULL;
    currentTask = ktask;
}

void createKernelTask(task_struct *task, void (*func)(void)){

    task->stack = kmalloc();
    task->init = 1;
    task->stack[510] = (uint64_t)func;
    task->rsp = (uint64_t)&task->stack[510];
    task->user_rip = (uint64_t)&func1;
    task->cr3 = (uint64_t)getKernelPML4();
    task->no_of_children = 0;
    task->next = NULL;
    task->nextChild = NULL;
    task->type = TASK_KERNEL;
    task->state = TASK_STATE_RUNNING;
    addTaskToReady(task);
}

void createUserProcess(task_struct *user_task){
    uint64_t userbase = VIRBASE;
    user_task->type = TASK_USER;
    user_task->state = TASK_STATE_RUNNING;
    user_task->cr3 = (uint64_t)kmalloc();
    user_task->no_of_children = 0;
    user_task->next = NULL;
    user_task->nextChild = NULL;
    user_task->stack = kmalloc();
    //user_task->rip = (uint64_t)&userFunc;

    uint64_t curr_rsp;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(curr_rsp));

    curr_rsp = (curr_rsp>>12)<<12;
    kmemcpy(user_task->stack, (uint64_t *)curr_rsp, PAGE_SIZE);

//    user_task->stack[499] = (uint64_t)(MM_STACK_START-0x10);
//    user_task->rsp = (uint64_t )&user_task->stack[499];

    //user_task->rsp = (uint64_t )(MM_STACK_START-0x10);

    user_task->kernInitRSP = (uint64_t)&user_task->stack[510];
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
    kmemcpy((void*)userPage,(void*)kernPage ,PAGE_SIZE);


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
//    user_task->rsp = (uint64_t)&user_task->stack[499];
//
//    // map user page rip
//    map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
//    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr);
//    user_task->rip = userbase | (((uint64_t)&userFunc) & ~ADDRESS_SCHEME); //RM: pop address offset
//    userbase+=0x1000;

    userPage = (uint64_t)kmalloc();
    map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr);
    userbase+=0x1000;

    user_task->mm = (mm_struct*)userPage;
    user_task->mm->v_addr_pointer = userbase;
    user_task->mm->vma_cache = NULL;

    // map kernel
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
    kprintf("User process ready, kernFunc: %x, ring3Func: %x\n",&userFunc,user_task->user_rip);
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

    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));
    ret = (ret>>12) <<12;
    set_tss_rsp((uint64_t *)ret);
    kernel_rsp = ret;
    //set_tss_rsp((void*)(user_task->kernInitRSP - 16));
    //kernel_rsp = (user_task->kernInitRSP - 16);

    setCR3((uint64_t*)user_task->cr3);
    __asm__ volatile("mov $0x23, %%ax"::);
    __asm__ volatile("mov %%ax, %%ds"::);
    __asm__ volatile("mov %%ax, %%es"::);
    __asm__ volatile("mov %%ax, %%fs"::);
    __asm__ volatile("mov %%ax, %%gs"::);

    __asm__ volatile("movq %0, %%rax"::"r"(user_task->user_rsp));
    __asm__ volatile("pushq $0x23");
    __asm__ volatile("pushq %rax");
    __asm__ volatile("pushfq");
    __asm__ volatile("popq %rax");
    __asm__ volatile("or $0x200, %%rax;":::);
    __asm__ volatile("pushq %rax");
    __asm__ volatile("pushq $0x2B");
    __asm__ volatile("pushq %0"::"r"(user_task->user_rip));
    __asm__ volatile("iretq");
}


pid_t sys_fork() {
    task_struct* parent = getCurrentTask();
    task_struct* child = getFreeTask();
    createUserProcess(child);
    child->init = parent->init;
    child->user_rip = parent->user_rip;
    child->rsp = parent->rsp;


    //copy the file descriptor list and increment reference count
    int i = 0;
    while( i < MAX_FD && parent->fd[i] != NULL) {
        FD* fd = (FD*) kmalloc_size(sizeof(FD));
        fd->perm = parent->fd[i]->perm;
        fd->filenode =  parent->fd[i]->filenode;
        fd->current_pointer = parent->fd[i]->current_pointer;
        fd->ref_count = ++parent->fd[i]->ref_count;
        i++;
    }

    if(copy_mm(parent,child) == -1){
        kprintf("error while copying task");
        return -1;
    }

    child->parent  = parent;
    child->ppid = parent->pid;

    if(parent->child_list == NULL)
        parent->child_list = child;
    else {
        child->nextChild = parent->child_list;
        parent->child_list = child;
    }
    parent->no_of_children++;


    //copy kernel stack;
    uint64_t rsp ;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(rsp));
    //aligning down
    rsp = (rsp>>12)<<12;
    kmemcpy(child->stack, (uint64_t *)rsp, PAGE_SIZE);
//    child->kernInitRSP = &child->stack[499];
//    //schedule the next process; parent will only run after child
//    schedule();

    return child->pid;
}

// use next child link instead of next
void removeChildFromParent(task_struct *parent, task_struct*child){
    task_struct* ptr = parent->child_list;
    task_struct* prevptr = NULL;
    while(ptr){
        if(ptr->pid == child->pid)
        {
            if(prevptr == NULL)
                parent->child_list = ptr->nextChild;
            else
                prevptr->nextChild = ptr->nextChild;
            --parent->no_of_children;
            return;
        }
        prevptr = ptr;
        ptr = ptr->nextChild;
    }
}

// add all children of given parent task to init list
void addChildrenToInitTask(task_struct *parentTask){
    if(parentTask == NULL){
        return;
    }

    task_struct* taskChildPtr = parentTask->child_list;

    while(taskChildPtr){
        if(kernel_idle_task->child_list == NULL)
            kernel_idle_task->child_list = taskChildPtr;
        else {
            taskChildPtr->nextChild = kernel_idle_task->child_list;
            kernel_idle_task->child_list = taskChildPtr;
        }
        ++kernel_idle_task->no_of_children;
        taskChildPtr = taskChildPtr->nextChild;
    }
}

void removeTaskFromRunList(task_struct *task){

    if(task == NULL)
        return;

    // remove from active
    task_struct *readyListPtr = gReadyList;
    task_struct* prevptr = NULL;
    while(readyListPtr){
        if(readyListPtr->pid == task->pid)
        {
            if(prevptr == NULL)
                gReadyList = readyListPtr->next;
            else
                prevptr->next = readyListPtr->next;
            return;
        }
        prevptr = readyListPtr;
        readyListPtr = readyListPtr->next;
    }

    // remove from blocked
    task_struct *blockedListPtr = gBlockedList;
    prevptr = NULL;
    while(blockedListPtr){
        if(blockedListPtr->pid == task->pid)
        {
            if(prevptr == NULL)
                gBlockedList = blockedListPtr->next;
            else
                prevptr->next = blockedListPtr->next;
            return;
        }
        prevptr = blockedListPtr;
        blockedListPtr = blockedListPtr->next;
    }
}

// this function is used to remove a task from ready queue and blocked queue, and add it to zombie queue
void moveTaskToZombie(task_struct *task){
    if(task == NULL)
        return;

    removeTaskFromRunList(task);
    // add to zombie
    task->state = TASK_STATE_ZOMBIE;
    addTaskToZombie(task);
}

void destroy_task(task_struct *task){
    // TODO: no code to clear mm struct yet

    // if children, add them to init task
    if(task->child_list){
        addChildrenToInitTask(task);
    }

    // remove task from parent
    if(task->parent){
        removeChildFromParent(task->parent,task);
    }
}

void killTask(task_struct *task){

    if(task == NULL || task->state == TASK_STATE_ZOMBIE)
        return;

    // user process can't kill kernel task
    if(task->type == TASK_KERNEL && currentTask->type != TASK_KERNEL) {
        return;
    }

    destroy_task(task);
    // make currentTask active as zombie, add to zombie taken care in schedule
    if(task == currentTask){
        task->state = TASK_STATE_KILLED;
        schedule();
    }

}

void removeTaskFromBlocked(task_struct* task){
    if(task == NULL)
        return;

    task_struct* prevptr = NULL;
    task_struct *blockedListPtr = gBlockedList;

    while(blockedListPtr){
        if(blockedListPtr->pid == task->pid) {
            if(prevptr == NULL)
                gBlockedList = blockedListPtr->next;
            else
                prevptr->next = blockedListPtr->next;
            return;
        }
        prevptr = blockedListPtr;
        blockedListPtr = blockedListPtr->next;
    }

}

