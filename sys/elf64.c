//
// Created by Shweta Sahu on 11/1/17.
//
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/procmgr.h>
#include <sys/mm.h>
#include <sys/kstring.h>
#include <sys/pmm.h>
#include <sys/common.h>


//loads segments from elf binary image
int load_elf_binary(Elf64_Ehdr* elf_header, task_struct* task, file_table* file, char *argv[],char * envp[]){

    int is_exe = 0;
   if(NULL == elf_header){
#ifdef ERROR_LOGS_ENABLE
        kprintf("elf_header is null\n");
#endif
        return 0;
    }

   //entry point
    task->user_rip = elf_header->e_entry;

    Elf64_Phdr* progHeader;

    vm_area_struct* vma;
    uint64_t * pml4_pointer = (uint64_t*)task->cr3;
    //for each entry in the program header table
    for(int i=0; i < elf_header->e_phnum ; i++){

        progHeader = (Elf64_Phdr*)((uint64_t)elf_header + elf_header->e_phoff) + i;

        if(progHeader->p_type == PT_LOAD && progHeader->p_memsz >= progHeader->p_filesz){
            is_exe=1;
            //ELF SECTIONS to be loaded in new virtual memory area
            //uint64_t * start_pointer =
            vma = do_mmap(task, progHeader->p_vaddr, progHeader->p_memsz, progHeader->p_flags, file,progHeader->p_offset);
            //.BSS memory handling
            if(progHeader->p_filesz <progHeader->p_memsz && vma!=NULL){
                    allocate_pages_to_vma(vma,&pml4_pointer);
                    memset((void *)vma->vm_start + progHeader->p_filesz, 0, progHeader->p_memsz - progHeader->p_filesz);

           }


        }
    }
    if(!is_exe){
#ifdef ERROR_LOGS_ENABLE
        kprintf("no Loadable section found: exit\n");
#endif
        return -1;
    }
    //allocate heap
    allocate_heap(task->mm);



    allocate_stack(task,argv, envp);

    //allocate page for e_entry address

    vm_area_struct* vm = find_vma(task->mm,task->user_rip);
    allocate_pages_to_vma(vm,&pml4_pointer);

//    vm = find_vma(task->mm,0x60117c);
//    allocate_pages_to_vma(vm,&pml4_pointer);

    return 1;

}

int load_elf_binary_by_name(task_struct* task, char* binary_name, char *argv[],char * envp[]){
    //kprintf("inside load_elf_binary_by_name\n");
    file_table* file = find_tar(binary_name);

    /* TODO: check in environ also*/
    if(file == NULL || file->type != FILE){
#ifdef ERROR_LOGS_ENABLE
        kprintf("file not found\n");
#endif
        return -1;
    }

    void* tmp = (void*)file->start;

    Elf64_Ehdr *elf_header = (Elf64_Ehdr*)(tmp+ sizeof(struct posix_header_ustar));

    //if file is not executable then return
    if(elf_header == NULL ){
#ifdef ERROR_LOGS_ENABLE
        kprintf("elf header is null\n");
#endif
        return -1;
    }
    if (elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F'){
#ifdef ERROR_LOGS_ENABLE
        kprintf("not executable\n");
#endif
        return -1;
    }

    if(task == NULL) {
        //kprintf("task is null; allocating new task\n");
        task = getFreeTask();
        createUserProcess(task);
    }
    //clear exisiting mm
    memset(task->mm,0, sizeof(mm_struct));

    int l = kstrlen(binary_name);
    kmemcpy(task->name, binary_name, l);
    return load_elf_binary(elf_header,task,file,argv,envp);
    //return 1;
}

