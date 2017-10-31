//
// Created by robin manhas on 10/21/17.
//

#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/kprintf.h>

uint64_t* pml_table = NULL; // storing only pml table instance globally

extern uint64_t videoOutBufAdd; // TODO: Update it in kprintf too.

uint64_t virtualMemBase = 0;

uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt)
{
    *cr3_reg = 0x0;
    *cr3_reg |= ((pwt << 3) & 0x08);
    *cr3_reg |= (pcd << 4);
    *cr3_reg |= (pml4e_add & 0xfffffffffffff000);
    return cr3_reg;
}

// RM: Init table for kernel
uint64_t* pageTablesInit(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart)
{
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    if(!pml_table){
        Page* page = allocatePage(); // not keeping track of page as this won't need freeing
        pml_table = (uint64_t*)page->uAddress;
    }

    uint16_t pml4Off = ((virPageStart>>39)&0x1ff);
    uint16_t pdpOff = ((virPageStart>>30)&0x1ff);
    uint16_t pdOff = ((virPageStart>>21)&0x1ff);

    Page* pdpPage = allocatePage();
    Page* pdPage = allocatePage();
    Page* ptPage = allocatePage();

    pdp = (uint64_t*)pdpPage->uAddress;
    value = (uint64_t)pdp;
    value |= (0x007);
    pml_table[pml4Off] = value;

    pd = (uint64_t*)pdPage->uAddress;
    value = (uint64_t)pd;
    value |= (0x007);
    pdp[pdpOff] = value;

    pt = (uint64_t*)ptPage->uAddress;
    value = (uint64_t)pt;
    value |= (0x007);
    pd[pdOff] = value;

    /* map the kernel from physbase to physfree */
    for(;phyPageStart<phyPageEnd; phyPageStart += 0x1000, virPageStart += 0x1000)
    {

        uint16_t ptOff = ((virPageStart>>12)&0x1ff);
        uint64_t entry = phyPageStart;
        entry |= (0x007);
        pt[ptOff] = entry;
        if(phyPageStart == 0x200000){
            kprintf("phybase pml:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
        }
       // kprintf("pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
    }

    return pml_table;
}

static void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr)
{
    if(paddr == 0xb8000){
        kprintf("mapping video pointer\n");
    }
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    uint16_t pml4Off = ((vaddr>>39)&0x1ff);
    uint16_t pdpOff = ((vaddr>>30)&0x1ff);
    uint16_t pdOff = ((vaddr>>21)&0x1ff);
    uint16_t ptOff = ((vaddr>>12)&0x1ff);

    uint64_t pml4_entry = pml_table[pml4Off];
    if(pml4_entry & PTE_P)
        pdp = (uint64_t*)(pml4_entry & ADD_SCHEME);
    else{
        Page* pdpPage = allocatePage();
        pdp = (uint64_t*)pdpPage->uAddress;
        value = (uint64_t)pdp;
        value |= (0x007);
        pml_table[pml4Off] = value;
    }

    uint64_t pdpt_entry = pdp[pdpOff];
    if(pdpt_entry & PTE_P)
        pd = (uint64_t*)(pdpt_entry & ADD_SCHEME);
    else{
        Page* pdPage = allocatePage();
        pd = (uint64_t*)pdPage->uAddress;
        value = (uint64_t)pd;
        value |= (0x007);
        pdp[pdpOff] = value;
    }

    uint64_t pdt_entry = pd[pdOff];
    if(pdt_entry & PTE_P)
        pt = (uint64_t*)(pdt_entry & ADD_SCHEME);
    else{
        Page* ptPage = allocatePage();
        pt = (uint64_t*)ptPage->uAddress;
        value = (uint64_t)pt;
        value |= (0x007);
        pd[pdOff] = value;
    }

    value = paddr;
    value |= (0x007);
    pt[ptOff] = value;
//    if(paddr == 0x200000){
//        kprintf("phybase pml:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
//    }
    //kprintf("IP pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
    return;
}

void setIdentityPaging(uint64_t max_phy,void *physfree)
{
    uint64_t pbaseAdd = ((uint64_t)physfree) & 0xfffffffffffff000;
    uint64_t vaddr = (IDENTITY_MAP_V | pbaseAdd);
    uint64_t paddr =  pbaseAdd;
    uint64_t max_phys = max_phy;
    //kprintf("Received max: %x, padd: %x, vadd: %x\n",max_phys,paddr,vaddr);
    for(; paddr <= max_phys; paddr += PAGE_SIZE, vaddr += PAGE_SIZE){
        map_virt_phys_addr(vaddr, paddr);
    }

    /* map the video memory physical address to the virtual address */
    kprintf("page mapping complete, reset vid ptr\n");
    map_virt_phys_addr((uint64_t)0xffffffff800b8000UL, 0xb8000UL);
    videoOutBufAdd = (uint64_t)0xffffffff800b8000UL;
    kprintf("how tf can still print ? %x\n",videoOutBufAdd);
    /* RM: debug code for page table entries, **DO NOT DELETE**
    uint16_t pml4Off = ((0xffffffff80200000UL >> 39)&0x1ff);
    uint16_t pdpOff = ((0xffffffff80200000UL >> 30)&0x1ff);
    uint16_t pdOff = ((0xffffffff80200000UL >> 21)&0x1ff);
    uint16_t ptOff = ((0xffffffff80200000UL >> 12)&0x1ff);
    uint64_t *pdpval = (uint64_t*)(pml_table[pml4Off] & ADD_SCHEME);
    uint64_t *pdval = (uint64_t*)(pdpval[pdpOff] & ADD_SCHEME);
    uint64_t *ptval = (uint64_t*)(pdval[pdOff] & ADD_SCHEME);
    kprintf("phybase pml:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdpval[pdpOff],pdval[pdOff],ptval[ptOff]);
    */

   // map_virt_phys_addr((0xFFFFFFFF80000000UL | (uint64_t) pFreeList),((uint64_t)pFreeList));
   // pFreeList = (Page*)(0xFFFFFFFF80000000UL | (uint64_t) pFreeList);

}

