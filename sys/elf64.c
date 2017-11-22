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
#include <sys/vmm.h>
#include <sys/elf64.h>
#include <sys/procmgr.h>


//void* get_elf_binary(char* binary_name) {
//    kprintf("start\n");
//    if(NULL == binary_name)
//        return NULL;
//
//    struct posix_header_ustar* start = (struct posix_header_ustar*)&_binary_tarfs_start;
//    struct posix_header_ustar* end = (struct posix_header_ustar*)&_binary_tarfs_end;
//
//    uint64_t* pointer = (uint64_t*)start;
//    uint64_t* end_pointer = (uint64_t*)end;
//    uint64_t size =0;
//    uint64_t header_size = sizeof(struct posix_header_ustar);
//    uint64_t tmp = 0;
//
//    //uint64_t *end = (uint64_t *)start;
//    //while(*end || *(end+1) || *(end+2)){
//    while(*pointer < *end_pointer){
//        start = (struct posix_header_ustar*)(&_binary_tarfs_start+ tmp);
//        pointer = (uint64_t *)start;
//        kprintf("loop: %s\n",start->name);
//        if(strcmp(start->name,"") == 0){
//            break;
//        }
//        if(strcmp(start->name,binary_name) == 0) {
//            kprintf("found: %s\n",binary_name);
//            return (void*)start;
//        }
//
//        else{
//            size = octalToDecimal(stoi(start->size));
//
//            if(size% header_size != 0){
//                tmp += (size  + (header_size - size %header_size)+ header_size);
//            }
//            else
//                tmp += (size + header_size);
//
//
//        }
//
//    }
//    kprintf("not found\n");
//    return NULL;
//}
//loads segments from elf binary image
int load_elf_binary(Elf64_Ehdr* elf_header, task_struct* task){

   if(NULL == elf_header){
        kprintf("elf_header is null\n");
        return 0;
    }
    //vm_area_struct* current_vm = NULL;
    //vm_area_struct* tmp = NULL;
    //entry point
    task->rip = elf_header->e_entry;

    Elf64_Phdr* progHeader;// = (Elf64_Phdr*)((uint64_t)elf_header + elf_header->e_phoff);
   // uint64_t* curr_cr3 = (uint64_t *)getCR3();
    //for each entry in the program header table
    for(int i=0; i < elf_header->e_phnum ; i++){

        progHeader = (Elf64_Phdr*)((uint64_t)elf_header + elf_header->e_phoff) + i;

        if(progHeader->p_type == PT_LOAD && progHeader->p_memsz >= progHeader->p_filesz){


            //ELF SECTIONS to be loaded in new virtual memory area
            //setCR3((uint64_t *)task->cr3);
            do_mmap(task, progHeader->p_vaddr, progHeader->p_memsz, progHeader->p_flags, NULL,0);

        }
    }
    //allocate heap
    allocate_heap(task->mm);

    //allocate stack
    //schedule process
    return 1;

}

int load_elf_binary_by_name(char* binary_name, char *argv[]){
    kprintf("inside load_elf_binary_by_name\n");
    void* tmp = find_file(binary_name);
    Elf64_Ehdr *elf_header = (Elf64_Ehdr*)(tmp+ sizeof(struct posix_header_ustar));

    //if file is not executable then return
    if(elf_header == NULL ){
        kprintf("elf header is null\n");
        return 0;
    }
    if (elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F'){
        kprintf("not executable\n");
        return 0;
    }

    task_struct* task = allocate_task(1);
    return load_elf_binary(elf_header,task);
    //return 1;
}

