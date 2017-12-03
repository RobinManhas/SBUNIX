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

typedef enum task_type {
    TASK_KERNEL = 1,
    TASK_USER = 2
}task_type;

typedef enum task_state {
    TASK_STATE_RUNNING = 1,
    TASK_STATE_BLOCKED = 2,
    TASK_STATE_ZOMBIE = 3,
    TASK_STATE_IDLE = 4,
    TASK_STATE_KILLED = 5,
    TASK_MAX = 6
}task_state;

//should not increse 4096 bytes
typedef struct task_struct{
    uint8_t init; // has val 1 when task is just created. Val made 0 when task about to execute (will have valid regs which can be saved during switch)
    uint16_t pid;
    uint16_t ppid; // parent's pid

    uint64_t rsp;  //kernel rsp
    uint64_t kernInitRSP;

    uint64_t user_rsp;
    uint64_t user_rip;
    uint64_t cr3;
    uint64_t* stack;

    task_type type;
    task_state state;
    struct task_struct *next;
    struct task_struct *nextChild;
    FD* fd[MAX_FD]; //can we save just id in int?
    mm_struct* mm;
    struct task_struct* parent;
    struct task_struct* child_list;
    uint8_t no_of_children;
    char* name;
} task_struct;

//task_struct* CURRENT_TASK;



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
    file_table* file;
    uint64_t file_offset;//file offset
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
    uint64_t v_addr_pointer;
};

uint16_t getFreePID();
void killTask(task_struct *task);
void threadInit();
void switch_to(task_struct *current, task_struct *next);
void createUserProcess(task_struct *user_task);
void createKernelInitProcess(task_struct *ktask, task_struct *startFuncTask);
void createKernelTask(task_struct *task, void (*start)(void));
void switch_to_user_mode(task_struct *oldTask, task_struct *user_task);
void schedule();
task_struct* getFreeTask();
void addTaskToReady(task_struct *readyTask);
void addTaskToBlocked(task_struct *blockedTask);
void addTaskToZombie(task_struct *zombieTask);
void removeTaskFromBlocked(task_struct* task);
void removeChildFromParent(task_struct *parent, task_struct*child);
void destroy_task(task_struct *task);
void removeTaskFromRunList(task_struct *task);
void moveTaskToZombie(task_struct *task);
task_struct* getCurrentTask();
pid_t sys_fork();
//task_struct* currentTask;

#endif //OS_PROCESSM_H



