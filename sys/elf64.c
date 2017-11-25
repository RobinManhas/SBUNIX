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
#include <sys/mm.h>
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
int load_elf_binary(Elf64_Ehdr* elf_header, task_struct* task, file_table* file){

    int is_exe = 0;
   if(NULL == elf_header){
        kprintf("elf_header is null\n");
        return 0;
    }

   //entry point
    task->rip = elf_header->e_entry;

    Elf64_Phdr* progHeader;

    //for each entry in the program header table
    for(int i=0; i < elf_header->e_phnum ; i++){

        progHeader = (Elf64_Phdr*)((uint64_t)elf_header + elf_header->e_phoff) + i;

        if(progHeader->p_type == PT_LOAD && progHeader->p_memsz >= progHeader->p_filesz){
            is_exe=1;
            //ELF SECTIONS to be loaded in new virtual memory area
            //uint64_t * start_pointer =
            do_mmap(task, progHeader->p_vaddr, progHeader->p_memsz, progHeader->p_flags, file,progHeader->p_offset);

        }
    }
    if(!is_exe){
        kprintf("no Loadable section found: exit\n");
        return -1;
    }
    //allocate heap
    allocate_heap(task->mm);
    allocate_stack(task);

    //allocate page for e_entry address
    uint64_t * pml4_pointer = (uint64_t*)task->cr3;
    vm_area_struct* vm = find_vma(task->mm,task->rip);
    allocate_pages_to_vma(vm,&pml4_pointer);


    kprintf("elf loaded successfully\n");
    return 1;

}

int load_elf_binary_by_name(task_struct* task, char* binary_name, char *argv[]){
    kprintf("inside load_elf_binary_by_name\n");
    file_table* file = find_file(binary_name);
    if(file == NULL){
        kprintf("file not found\n");
        return -1;
    }
    void* tmp = (void*)file->start;
    if(file->type != FILE){
        kprintf("Not a file; exit\n");
        return -1;
    }
    Elf64_Ehdr *elf_header = (Elf64_Ehdr*)(tmp+ sizeof(struct posix_header_ustar));

    //if file is not executable then return
    if(elf_header == NULL ){
        kprintf("elf header is null\n");
        return -1;
    }
    if (elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F'){
        kprintf("not executable\n");
        return -1;
    }

    if(task == NULL) {
        kprintf("task is null; allocating new task\n");
        task = getFreeTask();
        createUserProcess(task);
    }

    return load_elf_binary(elf_header,task,file);
    //return 1;
}

