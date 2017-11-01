//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>

uint32_t ProcessIDMax = 0;

void runner(){ while(1);}

uint32_t getNextPID(){
    return ProcessIDMax++;
}

void createKernelProc(task_struct* task, uint64_t funcPtr){
    // TODO: fill task struct for kernel process here
    task->pid = getNextPID();
    task->state = TASK_STATE_INITIALIZED;
    kprintf("task created, PID: %d\n",task->pid);
}

void createInitProcess(){
    kprintf("Reached where we have to\n");
    task_struct* task = (task_struct*)kmalloc();
    createKernelProc(task, (uint64_t)runner);

    return;
}