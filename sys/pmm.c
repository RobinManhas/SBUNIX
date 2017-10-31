//
// Created by robin manhas on 10/18/17.
//

#include <sys/pmm.h>
#include <sys/kprintf.h>

static int totalPageCount;
static struct smap_t smapGlobal[2];
uint64_t maxPhyRegion;

void* memset(void* ptr, int val, unsigned int len){
    unsigned char *p = ptr;
    while(len > 0)
    {
        *p = val;
        p++;
        len--;
    }
    return(p);
}

uint64_t phyMemInit(uint32_t *modulep, void *physbase, void **physfree) {

    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for (smap = (struct smap_t *) (modulep + 2);
         smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            // RM: type 1 is available memory http://www.brokenthorn.com/Resources/OSDev17.html

            if (smap->base == 0) // base 0 meaning beginning
            {
                //totalPageCount=smap->length/PAGE_SIZE;
                //kprintf("Lower mem pages = %d\n",totalPageCount); RM: not doing anything for pages below phsyfree
            }
            else
            {
                // TODO *IMPORTANT*: Currently we store just the last contiguous block in high mem, need to do for all
                totalPageCount = ((smap->base + smap->length) - (uint64_t) *physfree) / PAGE_SIZE;
                kprintf("Higher mem pages = %d\n", totalPageCount);
                smapGlobal[1].base = smap->base;
                smapGlobal[1].length = smap->length;
                smapGlobal[1].type = smap->type;
                break;
            }

            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
        }
    }

    // RM: as per lecture, allocate this space just above physfree, move physfree after this allocation
    Page *pageUpdateList = (Page *) *physfree;
    pFreeList = (Page *) *physfree;
    int totalSizeUsed = totalPageCount * sizeof(Page);
    uint64_t newPhysFree = (uint64_t) (*physfree) + (totalSizeUsed);

    uint64_t ptr = ((newPhysFree + PAGE_SIZE) & 0xfffffffffffff000); // flush flags
    kprintf("First ptr %x\n", ptr);
    Page *pre = NULL;
    maxPhyRegion = smapGlobal[1].base + smapGlobal[1].length;
    for (; ptr < (maxPhyRegion); ptr += PAGE_SIZE) {
        pageUpdateList->uAddress = ptr;
        pageUpdateList->sRefCount = 0;
        pageUpdateList->pNext = NULL;
        if (pre != NULL) {
            pre->pNext = (Page *) ((uint64_t) pageUpdateList);
        }
        pre = pageUpdateList;
        memset((void*)pageUpdateList->uAddress, 0, PAGE_SIZE);
        pageUpdateList += sizeof(Page);
    }

    (*physfree) = (void*)newPhysFree;
    kprintf("new physfree: %x, max: %x\n", *physfree,maxPhyRegion);
    return maxPhyRegion;
}



// allocate a single page
Page* allocatePage(){
    Page* page = NULL;
    page = pFreeList;
    if(pFreeList){
        pFreeList = pFreeList->pNext;
        //pFreeList->pPrev = NULL;
        page->pNext = NULL; // Prev was already null for 1st page of free list
        ++page->sRefCount;
    }
    else{
        kprintf("Out of physical pages..\n");
    }

    return page;
}

void deallocatePage(Page* page){
    if(page){
        if(0 == (--page->sRefCount)){
            freePage(page);
        }
    }
}

void freePage(Page* page){
    memset((void*)page->uAddress,0,PAGE_SIZE);
    page->sRefCount = 0; // ideally not required but setting everything here.
    //page->pPrev = NULL;
    page->pNext = pFreeList;
    pFreeList = page;
}