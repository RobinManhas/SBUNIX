//
// Created by Shweta Sahu on 11/18/17.
//
#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/kstring.h>
#include <sys/vmm.h>
#include <sys/pmm.h>


//for munmap
//vm_area_struct *vma_free_list = NULL;


vm_area_struct* find_vma(mm_struct* mm, uint64_t addr){
    vm_area_struct *vma = NULL;
    if (mm) {
        vma = mm->vma_cache;
        if(vma && vma->vm_end > addr && vma->vm_start <= addr)
            return vma;

        vma = NULL;
        vm_area_struct* vm_pointer = mm->vma_list;
        while (vm_pointer->vm_next) {
            if (vma && vma->vm_end > addr && vma->vm_start <= addr)
                return vma;
            vma = vma->vm_next;
        }
        if (vma)
            mm->vma_cache = vma;
    }
    return vma;
}

vm_area_struct* add_vma_at_last(mm_struct* mm, uint64_t len, uint64_t flags, file *file, uint64_t offset, int extend) {
    uint64_t start_addr = 0;
    if (mm == NULL) {
        kprintf("mm not assigned");
        return NULL;
    }
    vm_area_struct *new_vma = allocate_vma(start_addr, start_addr + len, flags, file, offset);
    vm_area_struct *pointer = mm->vma_list;
    if (pointer == NULL) {
        mm->vma_list = new_vma;
        return new_vma;
    }
    while (pointer->vm_next != NULL)
        pointer = pointer->vm_next;

    //checking if the vma can be extended
    if (extend == 1 && pointer->vm_flags == flags) {
        pointer->vm_end = pointer->vm_end + len;
        return NULL;

    }

    //aligning to PAGESIZE
    start_addr = (uint64_t) ((((pointer->vm_end - 1) >> 12) + 1) << 12);


    pointer->vm_next = new_vma;
    mm->total_vm++;
    return new_vma;

}

vm_area_struct* allocate_vma(uint64_t start_addr, uint64_t end_addr, uint64_t flags, file *file,uint64_t offset){
    vm_area_struct *vma = NULL;//=get_free_vma_struct

    vma = (vm_area_struct*) umalloc_size(sizeof(vm_area_struct));
    vma->vm_start       = start_addr;
    vma->vm_end         = end_addr;
    vma->vm_next        = NULL;
    vma->vm_flags       = flags;
    vma->file  = file;
    vma->vm_offset = offset;
    return vma;

}

int copy_mm(task_struct* parent_task, task_struct* child_task) {

    if(parent_task == NULL || child_task == NULL || parent_task->mm == NULL || child_task->mm == NULL){
        kprintf(" mm not allocated");
        return 0;
    }
    if(parent_task->mm->vma_list == NULL){
        kprintf("no vma in parent");
        return 0;
    }

    memcpy((void *) child_task->mm, (void *) parent_task->mm, sizeof(mm_struct));

    child_task->mm->vma_list = NULL;
    vm_area_struct *child_vm_pointer = NULL;
    vm_area_struct *parent_vm_pointer = parent_task->mm->vma_list;
    uint64_t* parent_cr3 = (uint64_t *)parent_task->cr3;
    uint64_t* child_cr3 = (uint64_t *)child_task->cr3;

    while (parent_vm_pointer) {
        uint64_t start = parent_vm_pointer->vm_start;
        uint64_t end = parent_vm_pointer->vm_end;


        vm_area_struct *new_vma = allocate_vma(start, end, parent_vm_pointer->vm_flags, parent_vm_pointer->file,
                                               parent_vm_pointer->vm_offset);
        if (child_task->mm->vma_list == NULL) {
            child_task->mm->vma_list = new_vma;
            child_vm_pointer = new_vma;

        } else {
            child_vm_pointer->vm_next = new_vma;
            child_vm_pointer = child_vm_pointer->vm_next;
        }

        // COW
        uint64_t page_flags;
        uint64_t *page_phy_add;
        while (start < end) {
            setCR3(parent_cr3);
            //check??
            page_phy_add = get_pt_entry(start,1);
             if(*page_phy_add & PTE_P) {
                 //removing write flag
                 *page_phy_add = *page_phy_add & (~PTE_W);
                 //setting cow bits
                 *page_phy_add = *page_phy_add | PTE_COW;

                 page_flags = *page_phy_add & (~ADDRESS_SCHEME);
                 *page_phy_add = *page_phy_add & ADDRESS_SCHEME;

                 setCR3(child_cr3);
                 map_virt_phys_addr_cr3(start, *page_phy_add, page_flags,1);
             }
            start = start + PAGE_SIZE;// for multiple pages
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
uint64_t do_mmap(task_struct* task, uint64_t addr, uint64_t len, uint64_t flags, struct file *file, uint64_t offset){
    uint64_t start_addr ;

    start_addr = findFreeVmaSlot(task->mm, addr,len);
    //if free slot found in middle;
    if(start_addr !=0){
        if(extendVma(flags,len)==1)
            return start_addr;

        vm_area_struct *new_vm;

        new_vm = allocate_vma(start_addr, start_addr + len, flags, file,offset);

        kprintf("\n node start %p, end%p fd %d", new_vm->vm_start, new_vm->vm_end, new_vm->file);


        vm_area_struct* curr = task->mm->vma_list;
        if(curr == NULL){
            curr = new_vm;
        }
        else {
            vm_area_struct *last = NULL;
            int found = 0;
            while (curr->vm_next != NULL) {
                last = curr;
                curr = curr->vm_next;

                if ((last->vm_end < start_addr) && (curr->vm_start > (start_addr + len))) {
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

        return start_addr;

    }else
        return add_vma_at_last(task->mm,len, flags,file,offset,1)->vm_start;


}

uint64_t allocate_heap(mm_struct* mm) {
    // at heap at last
    uint64_t start_addr = add_vma_at_last(mm, 0, PTE_W, NULL, 0, 0)->vm_start;

    mm->start_brk = start_addr;
    mm->brk = start_addr;
    return start_addr;
}

void allocate_pages_to_vma(vm_area_struct* vma, int isUser){
    uint64_t start = vma->vm_start;
    uint64_t end = vma->vm_end;
    int no_of_pages = (end-start)/PAGE_SIZE;
    if((end-start)%PAGE_SIZE > 0)
        no_of_pages++;

    uint64_t phy_new;
    while(no_of_pages>=1 && start<end) {
        phy_new = allocatePage();
        map_virt_phys_addr_cr3( start, phy_new, vma->vm_flags, isUser);
        no_of_pages--;
        start = start+PAGE_SIZE;
    }

}


void* umalloc_size(uint64_t size) {
    uint64_t noOfPages = 0;

    if (size < PAGE_SIZE){
        noOfPages = 1;
    }
    else{
        noOfPages = size/PAGE_SIZE;

        if (size%PAGE_SIZE >0)
            noOfPages++;
    }
    uint64_t pagesAdd[noOfPages];
    uint64_t virAdd =0;
    for(int i = 0;i<noOfPages; i++) {
        pagesAdd[i] = allocatePage();
    }

    for(int i = 0;i<noOfPages; i++) {

        virAdd = returnVirAdd(pagesAdd[i],VMAP_BASE_ADD,1);
        map_virt_phys_addr(virAdd,((uint64_t)pagesAdd[i] & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
    }

    return (void*)returnVirAdd(pagesAdd[0],VMAP_BASE_ADD,1);
}





