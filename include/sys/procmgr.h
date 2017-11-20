//
// Created by robin manhas on 10/28/17.
//

#ifndef OS_PROCESSM_H
#define OS_PROCESSM_H

#define MAX_FD 50

#include <sys/defs.h>
#include <sys/tarfs.h>



typedef struct vm_area_struct vm_area_struct;
typedef struct mm_struct mm_struct;

//should not increse 4096 bytes
typedef struct {
    uint64_t pid;
    uint64_t rip;
    uint64_t rsp;
    uint64_t cr3;
    uint64_t stack[501];
    FD* fd[MAX_FD]; //can we save just id in int?
    mm_struct* mm;
    uint64_t *elf;
} task_struct;

task_struct* CURRENT_TASK;
struct file{
    uint64_t   start;
    uint64_t   pgoff;
    uint64_t   size;
    uint64_t   bss_size;
};
typedef struct file file;

enum vma_flag {
    NONE,  //no permission
    X,     //execute only
    W,     //write only
    WX,    //write execute
    R,     //read only
    RX,    //read execute
    RW,    //read write
    RWX    //read write execute
};
struct vm_area_struct{
    mm_struct* vm_mm;
    uint64_t vm_start;
    uint64_t vm_end;
    vm_area_struct* vm_next;
    uint64_t vm_flags;//protection or permission
    file* file;
    uint64_t vm_offset;//file offset
};


struct mm_struct {
    struct vm_area_struct * vma_list; //list of  memory areas
    struct vm_area_struct * vma_cache;
    uint64_t free_area_cache;
    int mm_users;
    int mm_count; // primary usage counter
    int total_vm; //number of memory areas
    uint64_t start_code;
    uint64_t  end_code;
    uint64_t start_data;
    uint64_t end_data;
    uint64_t start_brk;//start address of heap
    uint64_t brk;       //final address of heap
    uint64_t start_stack;
    uint64_t start_mmap;
    uint64_t arg_start;
    uint64_t arg_end;
    uint64_t env_start;
    uint64_t env_end;
    uint64_t rss;       //pages allocated
    uint64_t locked_vm;//number of locked pages
    uint64_t flags;
};


void threadInit();
void switch_to(task_struct *current, task_struct *next);
void init_switch_to(task_struct *current, task_struct *next);
void createUserProcess();
void switch_to_user_mode(task_struct *user_task);

task_struct* currentTask;
task_struct* allocate_task(int is_user_task);


#endif //OS_PROCESSM_H



