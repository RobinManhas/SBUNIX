//
// Created by robin manhas on 10/28/17.
//

#ifndef OS_PROCESSM_H
#define OS_PROCESSM_H

#define MAX_FD 100

#include <sys/defs.h>
#include <sys/tarfs.h>



typedef struct vm_struct vm_struct;

typedef struct {
    uint64_t pid;
    uint64_t rip;
    uint64_t rsp;
    uint64_t cr3;
    uint64_t stack[501];
    FD* fd[MAX_FD];
    vm_struct* vm_head;
} task_struct;

struct vm_struct{
    task_struct* task;
    uint64_t vm_start;
    uint64_t vm_end;
    vm_struct* vm_next;
    uint64_t file;

};


void threadInit();
void switch_to(task_struct *current, task_struct *next);
void init_switch_to(task_struct *current, task_struct *next);
void createUserProcess();
void switch_to_user_mode(task_struct *user_task);

task_struct* currentTask;


#endif //OS_PROCESSM_H



