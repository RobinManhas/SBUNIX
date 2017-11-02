//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>

uint32_t ProcessIDMax = 0;

void runner(){
    int c = 0;
    while(1){
        if(c == 10000){
            kprintf("runner running\n");
            c = 0;
        }
        c++;
    };
}

uint32_t getNextPID(){
    return ProcessIDMax++;
}

/* Task structure value
    uint64_t pid;
    uint64_t rip;
    uint64_t rsp;
    uint64_t cr3;
    task_state state;
    int exit_status;
    uint64_t kernel_stack[MAX_STACK_SIZE];
    struct stTaskInfo* parentTask;
    struct stTaskInfo* nextTask;
 */

void createKernelProc(task_struct* task, uint64_t funcPtr){

    task->kernel_stack[127] = 0x10;
    task->kernel_stack[126] = (uint64_t)&task->kernel_stack[127];
    task->kernel_stack[125] = 0x20202;
    task->kernel_stack[124] = 0x08;
    task->kernel_stack[123] = funcPtr;

    /* Pretend that the GP registers and one function call is also on the stack */
    int indx = 122;
    for (; indx>105; indx--) {
        task->kernel_stack[indx] = 0x0;
    }
    task->rsp = (uint64_t)&task->kernel_stack[105];

    //task->rip = funcPtr;
    //task->time_slices = DEFAULT_TIME_SLICE;
    //task->rflags = DEFAULT_FLAGS;
    task->nextTask = NULL;
    task->parentTask = NULL;
    task->pid = getNextPID();
//    task->vm_head = NULL;
//    task->waiting = 0;
//    task->waiton_spec_child_exit = -1;
//    task->num_children = 0;
//    task->wait_time_slices = 0;
//    task->parent_task = NULL;
//    /* Set up the process address space */
//    /* This has to be aligned on 0x1000 boundaries and need the physical address */
//    phys_vir_addr* page_addr = get_free_phys_page();
//    pml4_e* pml_entries_ptr = (pml4_e*)page_addr->vir_addr;
//
//    /* Map the higher memory by copying the PML4E*/
//    int i = 0;
//
//    /* Create blank PML4E */
//    for(i=0; i<512; i++){
//        if (i==PML4_REC_SLOT) {
//            /* The recursive mapping */
//            create_pml4_e(&pml_entries_ptr[i], (u64int)page_addr->phys_addr, 0x0, 0x07, 0x00);
//        } else {
//            pml_entries_ptr[i] = pml_entries[i];
//
//        }
//    }
//    cr3_reg process_cr3;
//    create_cr3_reg(&process_cr3, (u64int)page_addr->phys_addr, 0x00, 0x00);
//    task->cr3_register = process_cr3;
//
//    /* Add to the ready list */
//    add_to_ready_list(task);
}

void createInitProcess(){
    kprintf("Reached where we have to\n");
    task_struct* task = (task_struct*)kmalloc();
    createKernelProc(task, (uint64_t)runner);

    return;
}