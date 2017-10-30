//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>

void runner(){ while(1);}

uint32_t getNextPID(){
    return ProcessIDMax++;
}

void createKernelProc(task_struct* task, uint64_t funcPtr){
    // TODO: fill task struct for kernel process here
    task->pid = getNextPID();
    task->state = TASK_STATE_INITIALIZED;
}

void createInitProcess(){
    task_struct* task = (task_struct*)kmalloc(sizeof(task_struct));
    createKernelProc(task, (uint64_t)runner);
}