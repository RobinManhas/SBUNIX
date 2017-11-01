//
// Created by robin manhas on 10/18/17.
//

#include <sys/pmm.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>

// Important note regarding addressing
// Page descriptor list gets mapped above KERNBASE in virtual addressing
// Normal pages get mapped as VMAP_BASE

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
    pDirtyPageList = NULL;

    kprintf("new physfree: %x, max: %x\n", *physfree,maxPhyRegion);
    return maxPhyRegion;
}



/* allocate a single page
 * Returns an empty PHYSICAL PAGE address
 *
 */
uint64_t allocatePage(){
    uint64_t ret = 0;
    Page* page = NULL;
    page = pFreeList;
    if(pFreeList){
        //pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList->pNext,KERNBASE_ADD,1);
        pFreeList = pFreeList->pNext;
        page->pNext = NULL; // Prev was already null for 1st page of free list
        ++page->sRefCount;
        ret = page->uAddress;

        // update freelist global var to virtual, if prev ptr was virtual address
        if((uint64_t)page > KERNBASE){
            pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList,KERNBASE_ADD,0);
        }

        //kprintf("allocate: %x, ofree: %x, nfree: %x\n",ret,page,pFreeList);
        addToDirtyPageList(page);
    }
    else{
        kprintf("Error: Out of physical pages..\n");
    }

    return ret;
}


void addToDirtyPageList(Page* page){
    page->pNext = pDirtyPageList;
    pDirtyPageList = page;
    //kprintf("dirty page added: %x\n",pDirtyPageList);
}

// RM TODO: needs handlings when switched to virtual address mode
void deallocatePage(uint64_t add){
    uint64_t phyAdd = add;
    if(add > VMAP_BASE)
        phyAdd = returnPhyAdd(add,VMAP_BASE_ADD,1);

    Page* pageIter = pDirtyPageList;
    Page* prevPage = NULL;
    //kprintf("deal add:%x , v:%x, p:%x\n",pageIter->uAddress,add,phyAdd);
    while(pageIter){
        if(pageIter->uAddress != phyAdd){
            prevPage = pageIter;
            if(add > VMAP_BASE)
                pageIter = (Page*)returnVirAdd((uint64_t)pageIter->pNext,KERNBASE_ADD,0);
            else
                pageIter = pageIter->pNext;
            continue;
        }
        else{
            break;
        }
    }

    if(0 == (--pageIter->sRefCount)){
        //kprintf("removing page from dirty: %x\n",pageIter);
        // clean page
        memset((void*)add,0,PAGE_SIZE);

        // remove from dirty list
        if(prevPage == NULL){ // at root
            pDirtyPageList = pageIter->pNext;
        }
        else{ // not root
            prevPage->pNext = pageIter->pNext;
        }

        pageIter->pNext = NULL;

        // add to free list
        pageIter->sRefCount = 0; // ideally not required but re-setting everything here.
        if(add > VMAP_BASE){
            pageIter->pNext = (Page*)returnPhyAdd((uint64_t)pFreeList,KERNBASE_ADD,0);
        }
        else{
            pageIter->pNext = pFreeList;
        }

        pFreeList = pageIter;
        //kprintf("inserted from dirty to free: %x\n",pFreeList);
    }
}
