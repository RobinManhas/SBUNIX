//
// Created by Shweta Sahu on 11/18/17.
//
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/kstring.h>
#include <sys/mm.h>
#include <sys/vmm.h>
#include <sys/pmm.h>


//for munmap
//vm_area_struct *vma_free_list = NULL;

void print_vma_boundaries(mm_struct* mm){
//    if(mm == NULL){
//        kprintf("ERROR: MM in null");
//        return;
//    }
//    vm_area_struct *vma= mm->vma_list;
//    while(vma){
//    kprintf("vma bounds:from: %x  to: %x\n",vma->vm_start,vma->vm_end);
//        vma=vma->vm_next;
//    }

}
vm_area_struct* find_vma(mm_struct* mm, uint64_t addr){
    vm_area_struct *vma = NULL;
    if (mm) {
        vma = mm->vma_cache;
        if(vma && vma->vm_end > addr && vma->vm_start <= addr)
            return vma;

        vma = mm->vma_list;
        while (vma) {
            if ( vma->vm_end > addr && vma->vm_start <= addr)
                break;
            vma = vma->vm_next;
        }
        if (vma)
            mm->vma_cache = vma;
    }
    if(vma == NULL){
        kprintf("vma not found for: %x\n",addr);
        print_vma_boundaries(mm);
    }

    return vma;
}

vm_area_struct* add_vma_at_last(mm_struct* mm, uint64_t len, uint64_t flags, file_table *file, uint64_t offset, int extend) {
    /*TODO: define a starting addrress*/
    uint64_t start_addr = mm->v_addr_pointer;
    if (mm == NULL) {
        kprintf("mm not assigned");
        return NULL;
    }
    vm_area_struct *pointer = mm->vma_list;
    vm_area_struct *new_vma;

    if (pointer == NULL) {
        new_vma = allocate_vma(start_addr, start_addr + len, flags, file, offset);
        new_vma->vm_mm = mm;
        mm->vma_list = new_vma;
        mm->total_vm++;
        return new_vma;
    }

   while (pointer->vm_next != NULL)
            pointer = pointer->vm_next;


    //checking if the vma can be extended
    if (extend == 1 && pointer->vm_flags == flags) {
        pointer->vm_end = pointer->vm_end + len;
        return pointer;

    }

    //aligning to PAGESIZE
    start_addr = (uint64_t) ((((pointer->vm_end - 1) >> 12) + 1) << 12);

    //kprintf("\nadding vma at last: %x\n",start_addr);
    new_vma = allocate_vma(start_addr, start_addr + len, flags, file, offset);
    new_vma->vm_mm = mm;

    pointer->vm_next = new_vma;
    mm->total_vm++;
    return new_vma;

}

vm_area_struct* allocate_vma(uint64_t start_addr, uint64_t end_addr, uint64_t flags, file_table *file,uint64_t offset){
    vm_area_struct *vma = NULL;//=get_free_vma_struct

    vma = (vm_area_struct*) kmalloc();
    vma->vm_start       = start_addr;
    vma->vm_end         = end_addr;
    vma->vm_next        = NULL;
    vma->vm_flags       = flags;
    vma->file  = file;
    vma->file_offset = offset;
    vma->type = VMA_TYPE_NORMAL;
    return vma;

}

int copy_mm(task_struct* parent_task, task_struct* child_task) {

    if(parent_task == NULL || child_task == NULL || parent_task->mm == NULL || child_task->mm == NULL){
        kprintf(" mm not allocated");
        return -1;
    }
    if(parent_task->mm->vma_list == NULL){
        kprintf("no vma in parent");
        return -1;
    }
    //uint64_t* parent_cr3   = (uint64_t *)parent_task->cr3;
    //uint64_t* child_cr3    = (uint64_t *)child_task->cr3;
    //uint64_t * parent_pml4_pointer = (uint64_t*)parent_task->cr3;
    //uint64_t * child_pml4_pointer = (uint64_t*)child_task->cr3;

    kmemcpy((void *) child_task->mm, (void *) parent_task->mm, sizeof(mm_struct));

    child_task->mm->vma_list = NULL;
    vm_area_struct *child_vm_pointer = NULL;
    vm_area_struct *parent_vm_pointer = parent_task->mm->vma_list;

    while (parent_vm_pointer) {
        uint64_t start = parent_vm_pointer->vm_start;
        uint64_t end = parent_vm_pointer->vm_end;

        vm_area_struct *new_vma = allocate_vma(start, end, parent_vm_pointer->vm_flags, parent_vm_pointer->file,
                                               parent_vm_pointer->file_offset);

        // ref count for file ??
//        if(new_vma->file){
//            new_vma->file->
//        }
        new_vma->vm_mm = parent_task->mm;
        if (child_task->mm->vma_list == NULL) {
            child_task->mm->vma_list = new_vma;
            child_vm_pointer = new_vma;

        } else {
            child_vm_pointer->vm_next = new_vma;
            child_vm_pointer = child_vm_pointer->vm_next;
        }

        uint64_t page_phy_add;
        if(parent_vm_pointer->type == VMA_TYPE_STACK){
            while (start < end) {
                page_phy_add = getPTEntry(end);
                if (!page_phy_add){
                    // this check acts (hopefully) as the breakpoint for copying auto-growing stack between
                    // our currently allocated ranges 0x555555554000  to: 0x7ffffffffff8
                    kprintf("Error: physical page entry not found: %x\n",end);
                    break;
                }
                if (page_phy_add & PTE_P) {
                    //removing write flag
                    page_phy_add = page_phy_add & (~PTE_W);
                    //setting cow bits
                    page_phy_add = page_phy_add | PTE_COW;
                    //incrementing reference count
                    Page* page = get_page(page_phy_add);
                    if(page)
                        page->sRefCount++;

                    setPTEntry(end,page_phy_add);

                }
                end = end - PAGE_SIZE;
            }
        }
        else{
            while (start < end) {

                page_phy_add = getPTEntry(start);
                if (!page_phy_add){
                    kprintf("Error: physical page entry not found: %x\n",start);
                    break;
                }

                if (page_phy_add & PTE_P) {
                    //removing write flag
                    page_phy_add = page_phy_add & (~PTE_W);
                    //setting cow bits
                    page_phy_add = page_phy_add | PTE_COW;
                    //incrementing reference count
                    Page* page = get_page(page_phy_add);
                    if(page)
                        page->sRefCount++;

                    setPTEntry(start,page_phy_add);

                }
                start = start + PAGE_SIZE;

            }
        }

        parent_vm_pointer = parent_vm_pointer->vm_next;
    }
    child_task->mm->total_vm = parent_task->mm->total_vm;

    return 1;
}


int isFree(mm_struct* mm, uint64_t addr, uint64_t len) {
    if (addr >= KERNBASE)
        return 0;

    vm_area_struct *pointer = NULL;

    for (pointer = mm->vma_list; pointer != NULL; pointer = pointer->vm_next) {

        if (addr < pointer->vm_start && (addr+len) > pointer->vm_start)
            return 0;

        if (addr < pointer->vm_end && (addr + len) > pointer->vm_end)
            return 0;
    }
    return 1;
}

uint64_t findFreeVmaSlot(mm_struct* mm, uint64_t addr,uint64_t len) {
    // chcek if free
    //always be at last
    //will modify the approach when implementing munmap
//    if (addr != 0x0)
//        if (isFree(mm,addr, len) == 1)
//            return addr;


    return 0;

}

//returns 1 if vma is extended
int extendVma(uint64_t flags,uint64_t len){
    //to be implemented
    //if flag is equal to the last vma flag,then last vma range is extended
    return 0;
}

//to be used by kernel to create a new linear address interval
uint64_t do_mmap(task_struct* task, uint64_t addr, uint64_t len, uint64_t flags, struct file_table *file, uint64_t offset){
    //uint64_t start_addr ;

    if(addr == 0)
        addr = findFreeVmaSlot(task->mm, addr,len);


    //if free slot found in middle;
    if(addr !=0){
        if(extendVma(flags,len)==1)
            return addr;

        vm_area_struct *new_vm;

        new_vm = allocate_vma(addr, addr + len, flags, file,offset);
        new_vm->vm_mm = task->mm;

        vm_area_struct* curr = task->mm->vma_list;
        if(curr == NULL){
            task->mm->vma_list = new_vm;
        }
        else {
            vm_area_struct *last = NULL;
            int found = 0;
            while (curr->vm_next != NULL) {
                last = curr;
                curr = curr->vm_next;

                if ((last->vm_end < addr) && (curr->vm_start > (addr + len))) {
                    found = 1;
                    break;
                }
            }

            if (found == 1) {
                last->vm_next = new_vm;
                new_vm->vm_next = curr;
            } else {
                curr->vm_next = new_vm;
            }
        }
        task->mm->total_vm++;
//        uint64_t* pml = (uint64_t*)task->cr3;
//        allocate_pages_to_vma(new_vm,&pml);

        return addr;

    }else
        return add_vma_at_last(task->mm,len, flags,file,offset,1)->vm_start;


}

uint64_t allocate_heap(mm_struct* mm) {
    // at heap at last
    uint64_t start_addr = add_vma_at_last(mm, 1, PTE_W, NULL, 0, 0)->vm_start;
    mm->start_brk = start_addr;
    mm->brk = start_addr;
    return start_addr;
}

int no_of_elements(char** arr) {
    if(arr == NULL)
        return 0;
    int l = 0;
    while(*arr++)
        l++;
    return l;
}

uint64_t allocate_stack(task_struct* task,char *argv[], char *envp[]) {

    task->mm->start_stack = MM_STACK_START;

    vm_area_struct* stack = allocate_vma(MM_STACK_END,MM_STACK_START,PTE_U_W_P,NULL,0);
    stack->type = VMA_TYPE_STACK;
    vm_area_struct* pointer = task->mm->vma_list;
    while (pointer->vm_next != NULL)
        pointer = pointer->vm_next;
    pointer->vm_next = stack;

    print_vma_boundaries(task->mm);


    allocate_single_page(task,MM_STACK_START);
    task->mm->start_stack = (uint64_t)MM_STACK_START;



    //set argv and envp
    //check & validate data


    uint64_t * task_cr3 = (uint64_t*)task->cr3;
    uint64_t * curr_cr3 = (uint64_t *)getCurrentTask()->cr3;
    setCR3(task_cr3);
    int argc_count = no_of_elements(argv);
    int envp_count = no_of_elements(envp);
    //kprintf("argc count: %d, envp count: %d\n",argc_count,envp_count);

    uint64_t * stack_top = (uint64_t*) task->mm->start_stack;
    uint64_t *arg_tmp[20];
    uint64_t *env_tmp[20];
    int i, l;
    //envp strings
    for (i=envp_count-1; i>=0; i--) {
        l = kstrlen(envp[i])+1;
        stack_top = (uint64_t*)((void*)stack_top - l);
        kmemcpy((char*)stack_top, envp[i], l);
        env_tmp[i] = stack_top;
    }
    //argv strings
    for (i=argc_count-1; i>=0; i--) {
        l = kstrlen(argv[i])+1;
        stack_top = (uint64_t*)((void*)stack_top - l);
        kmemcpy((char*)stack_top, argv[i], l);
        arg_tmp[i] = stack_top;
    }
    stack_top--;
    *stack_top = 0;
    // envp pointers
    for (i = envp_count-1; i >= 0; i--) {
        stack_top--;
        *stack_top = (uint64_t)env_tmp[i];
       // kprintf("env pointer:%p value: %s\n", stack_top, *stack_top);
    }

    stack_top--;
    *stack_top = 0;
   //argv pointers
    for (i = argc_count-1; i >= 0; i--) {
        stack_top--;
        *stack_top = (uint64_t)arg_tmp[i];
        //kprintf("arg pointer: %p value: %s\n", stack_top, *stack_top);
    }
    stack_top--;
    *stack_top = (uint64_t)argc_count;
    // Store the arg count
    kprintf("argc pointer %p; value: %d ; user_rsp: %p\n", stack_top, *stack_top, stack_top);

    // Reset stack pointer
    task->user_rsp = (uint64_t )(stack_top);

    setCR3(curr_cr3);

    return MM_STACK_START;
}

void allocate_single_page(task_struct* task, uint64_t addr){
    uint64_t * task_cr3 = (uint64_t*)task->cr3;
    uint64_t phy_page = allocatePage();
    uint64_t vir_page_addr_to_allocate = (addr>>12)<<12;
    map_user_virt_phys_addr(vir_page_addr_to_allocate, phy_page, &task_cr3);

}

void allocate_pages_to_vma(vm_area_struct* vma,uint64_t** pml_ptr){



    uint64_t start = vma->vm_start;
    uint64_t end = vma->vm_end;
    file_table* file = vma->file;
    uint64_t file_content_pointer = file->start + sizeof(struct posix_header_ustar);
    uint64_t bytes_to_copy =PAGE_SIZE;
    int no_of_pages = (end-start)/PAGE_SIZE;
    uint64_t last_page_data = (end-start)%PAGE_SIZE;
    if( last_page_data > 0) {
        no_of_pages++;
    }

    uint64_t phy_new;
    while(no_of_pages>=1 && start<end) {

        phy_new = allocatePage();
        map_user_virt_phys_addr(start,phy_new,pml_ptr);

        if(file != NULL && vma->file_offset < vma->vm_end){
            //copy data from file starting from offset
            //bytes_to_copy = PAGE_SIZE;
            if(no_of_pages == 1 && last_page_data>0){
                bytes_to_copy = last_page_data;
            }

            //is the code was executed by kernel; then set cr3 of this task
            setCR3(*pml_ptr);
            kmemcpy((uint64_t *)start,(uint64_t *)(file_content_pointer+vma->file_offset),bytes_to_copy);
            setCR3((uint64_t *)getCurrentTask()->cr3);
            vma->file_offset += bytes_to_copy;
        }

        no_of_pages--;
        start = start+bytes_to_copy;
    }
//    kprintf("virtual addre %x\n",(uint64_t**)vma->vm_start);
//    kprintf("getcr3 %x\n",getCR3());

}


///
//void* umalloc(uint64_t size) {
//    uint64_t no_of_pages = 0;
//    uint64_t *userPtr = (uint64_t*)CURRENT_TASK->cr3;
//
//    if (size < PAGE_SIZE){
//        no_of_pages = 1;
//    }
//    else{
//        no_of_pages = size/PAGE_SIZE;
//
//        if (size%PAGE_SIZE >0)
//            no_of_pages++;
//    }
//    uint64_t pages_addr[no_of_pages];
//    uint64_t vir_addr =0;
//    for(int i = 0;i<no_of_pages; i++) {
//        pages_addr[i] = returnPhyAdd(CURRENT_TASK->cr3,KERNBASE_OFFSET,1);
//    }
//
//    //uint64_t
//    for(int i = 0;i<no_of_pages; i++) {
//
//        vir_addr = get_next_virtual_page_addr_for_user();
//        map_user_virt_phys_addr(vir_addr,pages_addr[i],&userPtr);
//    }
//
//    return (void*)returnVirAdd(pages_addr[0],VMAP_BASE_ADD,1);
//}

uint64_t get_new_cr3_for_user_process(task_struct* task){

    uint64_t phyPage = allocatePage();
    short  addType =VMAP_BASE_ADD;
    uint64_t viPage = returnVirAdd(phyPage,addType,1);
    uint64_t *userPtr = (uint64_t*)task->cr3;

    map_user_virt_phys_addr(viPage, ((uint64_t)phyPage & ADDRESS_SCHEME), &userPtr);


    return viPage;


}




