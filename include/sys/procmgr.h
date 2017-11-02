//
// Created by robin manhas on 10/28/17.
//

#ifndef OS_PROCESSM_H
#define OS_PROCESSM_H

#include <sys/defs.h>

#define MAX_STACK_SIZE 128

enum taskstate{
    TASK_STATE_INITIALIZED,
    TASK_STATE_RUNNING,
    TASK_STATE_SLEEPING,
    TASK_STATE_ZOMBIE,
    TASK_STATE_MAX
};
typedef enum taskstate task_state;

// a combination of vm_struct and vmap_area
// http://elixir.free-electrons.com/linux/v4.0.6/source/include/linux/vmalloc.h#L31
struct vm_struct{
    uint8_t vm_type; //heap, data, code etc..
    uint64_t vm_start; // start, inclusive
    uint64_t vm_end;   // end, exclusive
    struct vm_struct *vm_next;      /* list of VMA's */
};
typedef struct vm_struct vm_struct;
// a node for the tasks linked list
struct stTaskInfo{
    uint64_t pid;
    uint64_t rip;
    uint64_t rsp;
    uint64_t cr3;
    task_state state;
    int exit_status;
    uint64_t kernel_stack[MAX_STACK_SIZE];
    struct stTaskInfo* parentTask;
    struct stTaskInfo* nextTask;
    vm_struct* vm_head;
};

typedef struct stTaskInfo task_struct; // RM: linux naming conv
uint32_t getNextPID();
void createInitProcess();
void createKernelProc(task_struct* task, uint64_t funcPtr);

#endif //OS_PROCESSM_H
