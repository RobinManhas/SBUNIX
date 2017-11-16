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
    map_virt_phys_addr(ret,((uint64_t)phyPage & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
    return (void*)ret;
}