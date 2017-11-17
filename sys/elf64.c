//
// Created by Shweta Sahu on 11/1/17.
//
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/kstring.h>
#include <sys/kprintf.h>
#include <sys/kmalloc.h>
#include <sys/util.h>
#include <sys/procmgr.h>


void* get_elf_binary(char* binary_name) {
    kprintf("start\n");
    if(NULL == binary_name)
        return NULL;

    struct posix_header_ustar* start = (struct posix_header_ustar*)&_binary_tarfs_start;
    struct posix_header_ustar* end = (struct posix_header_ustar*)&_binary_tarfs_end;

    uint64_t* pointer = (uint64_t*)start;
    uint64_t* end_pointer = (uint64_t*)end;
    uint64_t size =0;
    uint64_t header_size = sizeof(struct posix_header_ustar);
    uint64_t tmp = 0;

    //uint64_t *end = (uint64_t *)start;
    //while(*end || *(end+1) || *(end+2)){
    while(*pointer < *end_pointer){
        start = (struct posix_header_ustar*)(&_binary_tarfs_start+ tmp);
        pointer = (uint64_t *)start;
        kprintf("loop: %s\n",start->name);
        if(strcmp(start->name,"") == 0){
            break;
        }
        if(strcmp(start->name,binary_name) == 0) {
            kprintf("found: %s\n",binary_name);
            return (void*)start;
        }

        else{
            size = octalToDecimal(stoi(start->size));

            if(size% header_size != 0){
                tmp += (size  + (header_size - size %header_size)+ header_size);
            }
            else
                tmp += (size + header_size);


        }

    }
    kprintf("not found\n");
    return NULL;
}
//loads segments from elf binary image
int load_elf_binary(void* binary_start, task_struct* task){

   if(NULL == binary_start){
        kprintf("file is null\n");
        return -1;
    }
    vm_struct* current_vm = NULL;
    vm_struct* tmp = NULL;
    Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)binary_start;
    Elf64_Phdr* progHeader;// = (Elf64_Phdr*)((uint64_t*)elfHeader + elfHeader->e_phoff);

    //for each entry in the program header table
    for(int i=0; i < elfHeader->e_phnum ; i++){
        progHeader = (Elf64_Phdr*)((uint64_t*)elfHeader + elfHeader->e_phoff + elfHeader->e_phentsize*i);
        if(progHeader->p_type == PT_LOAD && progHeader->p_memsz >= progHeader->p_filesz){

            tmp = (vm_struct*)kmalloc(); // memory allocation
            tmp->vm_next = NULL;

           if( task->vm_head == NULL){
               task->vm_head = tmp;
           }
            else{
               current_vm->vm_next = tmp;
           }
            tmp->vm_start = progHeader->p_vaddr;
            tmp->vm_end = progHeader->p_vaddr + progHeader->p_memsz;
            //set vm type
            tmp->file =(uint64_t) progHeader;
           // call function to allocate physical memory
            //allocate_phys_memory(task,progHeader->p_memsz,progHeader->p_vaddr);
            current_vm = tmp;
            //set correct vm_type and file descriptor

        }
    }
    //pending items
    //map one page for the program's initial stack
    //allocate heap
    return 1;

}

int load_elf_binary_by_name(char* binary_name, task_struct* task){
    void* file = get_elf_binary(binary_name);
    return load_elf_binary(file,task);
}