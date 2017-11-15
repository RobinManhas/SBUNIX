//
// Created by robin manhas on 10/28/17.
//

#ifndef OS_PROCESSM_H
#define OS_PROCESSM_H

#include <sys/defs.h>

typedef struct {
    uint64_t pid;
    uint64_t rip;
    uint64_t rsp;
    uint64_t cr3;
    uint64_t stack[501];
} task_struct;

void threadInit();
void switch_to(task_struct *current, task_struct *next);
void init_switch_to(task_struct *current, task_struct *next);
void createUserProcess();
void switch_to_user_mode(task_struct *user_task);
#endif //OS_PROCESSM_H
