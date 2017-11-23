//
// Created by robin manhas on 10/18/17.
//
#include <sys/kmalloc.h>
#include <sys/pmm.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>

// Similar job to allocatePage(), but returns virtual address rather than physical
void* kmalloc(/*unsigned int size*/){ // TODO: extend implementation to support returning multiple pages
    /*if(size == 0 || size > PAGE_SIZE){
        kprintf("size not supported\n");
        return NULL;
    }*/

    uint64_t phyPage = allocatePage();
    uint64_t ret = returnVirAdd(phyPage,KERNBASE_OFFSET,1);
    map_virt_phys_addr(ret,((uint64_t)phyPage & ADDRESS_SCHEME),(uint64_t)PTE_W_P);
    return (void*)ret;
}

//void* umalloc(){
//
//    uint64_t phyPage = allocatePage();
//    uint64_t ret = returnVirAdd(phyPage,KERNBASE_OFFSET,1);
//    map_virt_phys_addr(ret,((uint64_t)phyPage & ADDRESS_SCHEME),PTE_U_W_P);
//    return (void*)ret;
//}

void* kmalloc_size(uint64_t size) {
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

        virAdd = returnVirAdd(pagesAdd[i],KERNBASE_OFFSET,1);
        map_virt_phys_addr(virAdd,((uint64_t)pagesAdd[i] & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
    }

    return (void*)returnVirAdd(pagesAdd[0],KERNBASE_OFFSET,1);
}
