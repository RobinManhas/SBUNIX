//
// Created by Shweta Sahu on 11/23/17.
//


#ifndef SBUNIX_MM_H
#define SBUNIX_MM_H

#include <sys/defs.h>
#include <sys/procmgr.h>



uint64_t do_mmap(task_struct* task, uint64_t addr, uint64_t len, uint64_t flags, struct file_table *file, uint64_t offset);

void allocate_pages_to_vma(vm_area_struct* vma,uint64_t** pml_ptr);

int load_elf_binary_by_name(task_struct* task, char* binary_name, char *argv[]);
int copy_mm(task_struct* parent_task, task_struct* child_task);
vm_area_struct* find_vma(mm_struct* mm, uint64_t addr);
uint64_t allocate_heap(mm_struct* mm);
uint64_t allocate_stack(task_struct* task);
vm_area_struct* allocate_vma(uint64_t start_addr, uint64_t end_addr, uint64_t flags, file_table *file,uint64_t offset);
#endif //SBUNIX_MM_H