//
// Created by robin manhas on 10/18/17.
//

#include <sys/pmm.h>
#include <sys/kprintf.h>

static int totalPageCount;
static struct smap_t smap_g[2];

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

int phyMemInit(uint32_t *modulep, void *physbase, void **physfree) {

    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for (smap = (struct smap_t *) (modulep + 2);
         smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            // RM: type 1 is available memory http://www.brokenthorn.com/Resources/OSDev17.html

            if (smap->base == 0) // base 0 meaning beginning
            {
                //totalPageCount=smap->length/PAGE_SIZE;
                //kprintf("Lower mem pages = %d\n",totalPageCount); RM: not doing anything for pages below phsyfree
            } else {
                totalPageCount = ((smap->base + smap->length) - (uint64_t) *physfree) / PAGE_SIZE;
                kprintf("Higher mem pages = %d\n", totalPageCount);
                smap_g[1].base = smap->base;
                smap_g[1].length = smap->length;
                smap_g[1].type = smap->type;
            }

            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
        }
    }

    // RM: as per lecture, allocate this space just above physfree, move physfree after this allocation
    // pPageList = (struct Page*)alloc(totalPageCount* sizeof(struct Page));
    //kprintf("old physfree: %x\n", *physfree);

    struct Page *pageUpdateList = (struct Page *) *physfree;
    pFreeList = (struct Page *) *physfree;
    int totalSizeUsed = totalPageCount * sizeof(struct Page);
    uint64_t newPhysFree = (uint64_t) (*physfree) + (totalSizeUsed);

    uint64_t ptr = ((newPhysFree + PAGE_SIZE) & 0xfffffffffffff000); // flush flags
    struct Page *pre = NULL;
    for (; ptr < (smap_g[1].base + smap_g[1].length); ptr += PAGE_SIZE) {
        pageUpdateList->uAddress = ptr;
        pageUpdateList->sRefCount = 0;
        pageUpdateList->pNext = NULL;
        if (pre != NULL) {
            pre->pNext = (struct Page *) ((uint64_t) pageUpdateList);
        }
        pre = pageUpdateList;
        memset((void*)pageUpdateList->uAddress, 0, PAGE_SIZE);
        pageUpdateList += sizeof(struct Page);
    }

    (*physfree) = (void*)newPhysFree;
    kprintf("new physfree: %x\n", *physfree);
    return 0;
}



// allocate a single page
struct Page* allocatePage(){
    struct Page* page = NULL;
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

void deallocatePage(struct Page* page){
    if(page){
        if(0 == (--page->sRefCount)){
            freePage(page);
        }
    }
}

void freePage(struct Page* page){
    memset((void*)page->uAddress,0,PAGE_SIZE);
    page->sRefCount = 0; // ideally not required but setting everything here.
    //page->pPrev = NULL;
    page->pNext = pFreeList;
    pFreeList = page;
}