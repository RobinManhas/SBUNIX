//
// Created by robin manhas on 10/18/17.
//

#include <sys/pmm.h>
#include <sys/kprintf.h>
#include <sys/vmm.h>
#include <sys/common.h>
// Important note regarding addressing
// Page descriptor list gets mapped above KERNBASE in virtual addressing
// Normal pages get mapped as KERNBASE

static int totalPageCount;
int totalFreePages;
int totalDirtyPages;
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

void printPageCountStats(){
    kprintf("free pages: %d, dirty pages: %d\n",totalFreePages,totalDirtyPages);
}

/*void *memcpy(void *dest, const void *src, uint64_t n)
{
    unsigned char *pd = (unsigned char *)dest;
    const unsigned char *ps = (unsigned char *)src;
    if ( ps < pd )
        for (pd += n, ps += n; n--;)
            *--pd = *--ps;
    else
        while(n--)
            *pd++ = *ps++;
    return dest;
}*/

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
#ifdef DEBUG_LOGS_ENABLE
                kprintf("Higher mem pages = %d\n", totalPageCount);
#endif
                smapGlobal[1].base = smap->base;
                smapGlobal[1].length = smap->length;
                smapGlobal[1].type = smap->type;
#ifdef DEBUG_LOGS_ENABLE
                kprintf("Available high Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
#endif
                break;
            }

        }
    }

    // RM: as per lecture, allocate this space just above physfree, move physfree after this allocation
    Page *pageUpdateList = (Page *) *physfree;
    pFreeList = (Page *) *physfree;
    int totalSizeUsed = totalPageCount * sizeof(Page);
    uint64_t newPhysFree = (uint64_t) (*physfree) + (totalSizeUsed);

    uint64_t ptr = ((newPhysFree + PAGE_SIZE) & 0xfffffffffffff000); // flush flags

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
        pageUpdateList += 1;
#ifdef ERROR_LOGS_ENABLE
        if((uint64_t)pageUpdateList > newPhysFree)
            kprintf("exceeded: %x, ptr:%x, size:%x \n",pageUpdateList,newPhysFree, sizeof(Page));
#endif
    }

    (*physfree) = (void*)newPhysFree;
    pDirtyPageList = NULL;

    totalFreePages = ((smapGlobal[1].base + smapGlobal[1].length) - (uint64_t) *physfree) / PAGE_SIZE;
    totalDirtyPages = 0;
#ifdef DEBUG_LOGS_ENABLE
    kprintf("new physfree: %x, max: %x\n", *physfree,maxPhyRegion);
#endif
    return maxPhyRegion;
}



/* allocate a single page
 * Returns an empty PHYSICAL PAGE address, duty of caller to convert to virtual or use as it is.
 *
 */
uint64_t allocatePage(){
    uint64_t ret = 0;
    Page* page = NULL;
    page = pFreeList;
    if(pFreeList){
        //pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList->pNext,KERNBASE_OFFSET,1);
        pFreeList = pFreeList->pNext;
        page->pNext = NULL; // Prev was already null for 1st page of free list
        ++page->sRefCount;
        ret = page->uAddress;

        /* Note: below code only maps the page descriptor to virtual, actual page isn't mapped here
         * This is because the page might actually be required to be mapped in virtual or */
        if((uint64_t)page > KERNBASE){
            map_virt_phys_addr(returnVirAdd((uint64_t)pFreeList,KERNBASE_OFFSET,1),((uint64_t)pFreeList & ADDRESS_SCHEME),PTE_W_P);
            pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList,KERNBASE_OFFSET,0);
        }

        //kprintf("allocate: %x, ofree: %x, nfree: %x\n",ret,page,pFreeList);
        addToDirtyPageList(page);
    }
    else{
#ifdef ERROR_LOGS_ENABLE
        kprintf("Error: Out of physical pages..\n");
#endif
    }

    --totalFreePages;
    return ret;
}


void addToDirtyPageList(Page* page){ // page address must be virtual
    page->pNext = pDirtyPageList;
    pDirtyPageList = page;
    ++totalDirtyPages;
    //kprintf("dirty page added: %x\n",pDirtyPageList->uAddress);
}

void addToFreePageList(Page *page){
    uint8_t usingVAddr = 0;

    if(page == NULL)
        return;
    
    if((uint64_t)pFreeList > KERNBASE)
        usingVAddr = 1;

    page->sRefCount = 0; // ideally not required but re-setting everything here.
    if(usingVAddr){
        page->pNext = (Page*)returnPhyAdd((uint64_t)pFreeList,KERNBASE_OFFSET,0);
    }
    else{
        page->pNext = pFreeList;
    }

    if(usingVAddr){
        pFreeList = (Page*)returnVirAdd((uint64_t)page,KERNBASE_OFFSET,0);
    }
    else{
        pFreeList = page;
    }

    ++totalFreePages;
//    kprintf("page added free list: %x, uadd: %x, ref: %d\n",page,page->uAddress,page->sRefCount);
//    kprintf("inserted from dirty to free: %x, uadd: %x\n",pFreeList,pFreeList->uAddress);
}

void removePageFromDirtyList(Page *page){
    uint8_t usingVAddr = 0;
    Page* pageIter = pDirtyPageList, *prevPage=NULL;
    if(page == NULL)
        return;

    if((uint64_t)pDirtyPageList > KERNBASE)
        usingVAddr = 1;

    while(pageIter){
        if(pageIter->uAddress != page->uAddress){
            prevPage = pageIter;
            if(usingVAddr)
                pageIter = (Page*)returnVirAdd((uint64_t)pageIter->pNext,KERNBASE_OFFSET,0);
            else{
                pageIter = pageIter->pNext;
            }
            continue;
        }
        else{
            //kprintf("page found in dirty list: %x, uadd: %x, ref: %d\n",page,page->uAddress,page->sRefCount);
            --totalDirtyPages;
            if(prevPage == NULL){ // at root
                if(usingVAddr)
                    pDirtyPageList = (Page*)returnVirAdd((uint64_t)pageIter->pNext,KERNBASE_OFFSET,0);
                else
                    pDirtyPageList = pageIter->pNext;

            }
            else{ // not root
                if(usingVAddr)
                    prevPage->pNext = (void*)returnVirAdd((uint64_t)pageIter->pNext,KERNBASE_OFFSET,0);
                else
                    prevPage->pNext = pageIter->pNext;

                pageIter->pNext = NULL;
            }
            break;
        }

    }

}

void updateCOWInfo(uint64_t vadd, uint64_t phyAdd){
    Page* pageIter = NULL;

    if(vadd == 0 || phyAdd == 0)
    {
#ifdef ERROR_LOGS_ENABLE
        kprintf("Error: address is 0 in updateCOWInfo\n");
#endif
        return;
    }

    pageIter = get_page(phyAdd);
    // reset COW bits if ref count is 1 and COW bits are set
    if(pageIter->sRefCount == 1 && !(phyAdd & PTE_W) && (phyAdd & PTE_COW))   {
        phyAdd = phyAdd | PTE_W;
        phyAdd = phyAdd &(~PTE_COW);
        setPTEntry(vadd,phyAdd);
    }
}

void deallocatePage(uint64_t virtualAddress){ // TODO: code not verified, check vaddr assignments when updating pointers
    uint8_t usingVAddr = 0;
    uint64_t phyAdd = 0,phyAddWithBits=0, alignedVAddress = 0;

    if(virtualAddress == 0)
    {
        //kprintf("Error: virtual address is 0\n");
        return;
    }

    phyAddWithBits = getPTEntry(virtualAddress);

    if(phyAddWithBits == 0){
#ifdef ERROR_LOGS_ENABLE
        kprintf("no physical address found for %x\n",virtualAddress);
#endif
        return;
    }

    phyAdd = phyAddWithBits & ADDRESS_SCHEME;

    //kprintf("dealloc v: %x, p: %x\n",virtualAddress,phyAdd);

    if((uint64_t)pDirtyPageList > KERNBASE)
        usingVAddr = 1;

    Page* pageIter = NULL;

    pageIter = get_page(phyAdd);

    if(pageIter == NULL){
        //kprintf("Error: no page found for v: %x, p: %x\n",virtualAddress,phyAdd);
        return;
    }

    //kprintf("got page: %x, uadd: %x, ref: %d\n",pageIter,pageIter->uAddress,pageIter->sRefCount);

    alignedVAddress = virtualAddress & ADDRESS_SCHEME; // RM: align down the received virtual address
    if(0 == (--pageIter->sRefCount)){
       // kprintf("removing page from dirty: %x\n",pageIter);
        // clean page
        if(usingVAddr)
            memset((void*)alignedVAddress,0,PAGE_SIZE);
        else
            memset((void*)pageIter->uAddress,0,PAGE_SIZE);

        //kprintf("after mem set align add: %d\n",alignPtr[0]);
        // remove from dirty list
        removePageFromDirtyList(pageIter);

        // add to free list
        addToFreePageList(pageIter);
    }

    if(pageIter->sRefCount == 1){
        updateCOWInfo(virtualAddress,phyAddWithBits);
    }
}

Page* get_page(uint64_t physicalAddress){
    uint8_t usingVAddr = 0;

    if(physicalAddress == 0)
    {
#ifdef ERROR_LOGS_ENABLE
        kprintf("Error: physical address is 0\n");
#endif
        return NULL;
    }
    uint64_t recvPhyAdd = physicalAddress & ADDRESS_SCHEME;
    Page* pageIter = pDirtyPageList;
    if(pageIter == NULL)
        return NULL;

    if((uint64_t)pDirtyPageList > KERNBASE)
        usingVAddr = 1;

    while(pageIter){
        if(pageIter->uAddress != recvPhyAdd){
            if(pageIter->pNext){
                if(usingVAddr)
                    pageIter = (Page*)returnVirAdd((uint64_t)pageIter->pNext,KERNBASE_OFFSET,0);
                else
                    pageIter = pageIter->pNext;
                continue;
            }
            else
            {
                //kprintf("Error: page: %x not found\n",physicalAddress);
                return NULL;
            }
        }
        else{
            return pageIter;
        }
    }
    return NULL;

}
