//
// Created by robin manhas on 10/18/17.
//
#include <sys/kmalloc.h>
#include <sys/pmm.h>

void* kmalloc(unsigned int size){
    if(pFreeList == NULL || size == 0)
        return NULL;

    struct Page* modifier = pFreeList;
    void* retAddress = (void*)pFreeList; // TODO: Return physical or virtual address ?
    int pages = size/PAGE_SIZE;
    if(size < PAGE_SIZE || (size - pages*PAGE_SIZE)>0){
        pages += 1;
    }

    for(int i=0;i<pages;i++){
        ++modifier->sRefCount;
        modifier = modifier->pNext;
        if(i == pages - 1)
            modifier->pNext = NULL;
    }

    pFreeList = modifier;
    return retAddress;

}