//
// Created by robin manhas on 10/18/17.
//

#ifndef OS_MEMMANAGER_H
#define OS_MEMMANAGER_H
#include "sys/defs.h"
#define PAGE_SIZE 4096

struct smap_t {
    uint64_t base, length;
    uint32_t type;
}__attribute__((packed)) *smap;

struct Page{
    short       sRefCount;
    uint64_t    uAddress; // can have ID here, 0 would translate to (physfree+0*4096), 1 as (physfree+1*4096), would save space
    struct Page *pNext;
};
typedef struct Page Page;

Page* pFreeList;
Page* pDirtyPageList;

void* memset(void* ptr, int val, unsigned int len);
uint64_t phyMemInit(uint32_t *modulep, void *physbase, void **physfree);
int pageListInit();
uint64_t allocatePage();
void deallocatePage(uint64_t page);
void addToDirtyPageList(Page* page);

#endif //OS_MEMMANAGER_H
