//
// Created by robin manhas on 10/28/17.
//

#ifndef OS_PROCESSM_H
#define OS_PROCESSM_H

#include <sys/defs.h>

#define KERN_STACK_SIZE 128

uint32_t ProcessIDMax = 0;

enum taskstate{
    TASK_STATE_INITIALIZED,
    TASK_STATE_RUNNING,
    TASK_STATE_SLEEPING,
    TASK_STATE_ZOMBIE,
    TASK_STATE_MAX
};
typedef enum taskstate task_state;

// a node for the tasks linked list
struct stTaskInfo{
    uint64_t pid;
    uint64_t rsp;
    task_state state;
    int exit_status;
    uint64_t kernel_stack[KERN_STACK_SIZE];
    struct stTaskInfo* parentTask;
    struct stTaskInfo* nextTask;

};

typedef struct stTaskInfo task_struct; // RM: linux naming conv
uint32_t getNextPID();
void createInitProcess();
void createKernelProc(task_struct* task, uint64_t funcPtr);

#endif //OS_PROCESSM_H
