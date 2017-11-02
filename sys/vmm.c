//
// Created by robin manhas on 10/21/17.
//

#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/kprintf.h>

// Important note regarding addressing
// Page descriptor list gets mapped above KERNBASE in virtual addressing
// Normal pages get mapped as KERNBASE

uint64_t* pml_table = NULL; // storing only pml table instance globally

extern uint64_t videoOutBufAdd; // TODO: Update it in kprintf too.

uint64_t virtualMemBase = 0;

// RM: Init table for kernel
uint64_t* pageTablesInit(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart)
{
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    if(!pml_table){
        pml_table = (uint64_t*)allocatePage(); // not keeping track of page as this won't need freeing
    }

    uint16_t pml4Off = ((virPageStart>>39)&0x1ff);
    uint16_t pdpOff = ((virPageStart>>30)&0x1ff);
    uint16_t pdOff = ((virPageStart>>21)&0x1ff);

    pdp = (uint64_t*)allocatePage();
    value = (uint64_t)pdp;
    value |= (0x007);
    pml_table[pml4Off] = value;

    pd = (uint64_t*)allocatePage();
    value = (uint64_t)pd;
    value |= (0x007);
    pdp[pdpOff] = value;

    pt = (uint64_t*)allocatePage();
    value = (uint64_t)pt;
    value |= (0x007);
    pd[pdOff] = value;

    for(;phyPageStart<phyPageEnd; phyPageStart += 0x1000, virPageStart += 0x1000)
    {

        uint16_t ptOff = ((virPageStart>>12)&0x1ff);
        uint64_t entry = phyPageStart;
        entry |= (0x007);
        pt[ptOff] = entry;

       // kprintf("pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
    }

    return pml_table;
}

void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr)
{
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    uint16_t pml4Off = ((vaddr>>39)&0x1ff);
    uint16_t pdpOff = ((vaddr>>30)&0x1ff);
    uint16_t pdOff = ((vaddr>>21)&0x1ff);
    uint16_t ptOff = ((vaddr>>12)&0x1ff);

    uint64_t pml4_entry = pml_table[pml4Off];
    if(pml4_entry & PTE_P){
        pdp = (uint64_t*)(pml4_entry & ADD_SCHEME);
    }
    else
    {
        pdp = (uint64_t*)allocatePage();
        value = (uint64_t)pdp;
        value |= (0x007);
        pml_table[pml4Off] = value;
    }
    if((uint64_t)pml_table > KERNBASE && (uint64_t)pdp < KERNBASE){
        pdp = (uint64_t*)returnVirAdd((uint64_t)pdp,KERNBASE_ADD,0);
    }

    uint64_t pdpt_entry = pdp[pdpOff];
    if(pdpt_entry & PTE_P){
        pd = (uint64_t*)(pdpt_entry & ADD_SCHEME);
    }
    else
    {
        pd = (uint64_t*)allocatePage();
        value = (uint64_t)pd;
        value |= (0x007);
        pdp[pdpOff] = value;
    }
    if((uint64_t)pml_table > KERNBASE && (uint64_t)pd < KERNBASE){
        pd = (uint64_t*)returnVirAdd((uint64_t)pd,KERNBASE_ADD,0);
    }

    uint64_t pdt_entry = pd[pdOff];
    if(pdt_entry & PTE_P){
        pt = (uint64_t*)(pdt_entry & ADD_SCHEME);
    }
    else
    {
        pt = (uint64_t*)allocatePage();
        value = (uint64_t)pt;
        value |= (0x007);
        pd[pdOff] = value;
    }

    if((uint64_t)pml_table > KERNBASE && (uint64_t)pt < KERNBASE){
        pt = (uint64_t*)returnVirAdd((uint64_t)pd,KERNBASE_ADD,0);
    }

    value = paddr;
    value |= (0x007);
    pt[ptOff] = value;

    //kprintf("IP pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
    return;
}

void mapPhysicalRangeToVirtual(uint64_t max_phy, void *physfree)
{
    uint64_t pbaseAdd = ((uint64_t)physfree) & ADD_SCHEME;
    uint64_t vaddr = (KERNBASE | pbaseAdd);
    uint64_t paddr =  pbaseAdd;
    uint64_t max_phys = max_phy;
    for(; paddr <= max_phys; paddr += PAGE_SIZE, vaddr += PAGE_SIZE){
        map_virt_phys_addr(vaddr, paddr);
    }

    kprintf("page mapping complete, reset vid ptr, free: %x\n",pFreeList);
    map_virt_phys_addr((uint64_t)0xffffffff800b8000UL, 0xb8000UL);
    videoOutBufAdd = (uint64_t)0xffffffff800b8000UL;

    // update vmem top ptr
    virtualMemBase = vaddr;

//    // RM: debug code for page table entries, **DO NOT DELETE**
//    uint64_t viradd = (KERNBASE | ((uint64_t) pFreeList & ADD_SCHEME));
//    uint16_t pml4Off = ((viradd >> 39)&0x1ff);
//    uint16_t pdpOff = ((viradd >> 30)&0x1ff);
//    uint16_t pdOff = ((viradd >> 21)&0x1ff);
//    uint16_t ptOff = ((viradd >> 12)&0x1ff);
//    uint64_t *pdpval = (uint64_t*)(pml_table[pml4Off] & ADD_SCHEME);
//    uint64_t *pdval = (uint64_t*)(pdpval[pdpOff] & ADD_SCHEME);
//    uint64_t *ptval = (uint64_t*)(pdval[pdOff] & ADD_SCHEME);
//    kprintf("freelist pml:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdpval[pdpOff],pdval[pdOff],ptval[ptOff]);


    // update pml global var
    pml_table = (uint64_t*)(KERNBASE | (uint64_t)pml_table);

    // update freelist global var
    uint64_t virAddTemp = returnVirAdd((uint64_t)pFreeList, KERNBASE_ADD, 1);
    map_virt_phys_addr(virAddTemp,((uint64_t)pFreeList & ADD_SCHEME));
    pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList, KERNBASE_ADD, 0);

    // update dirty list global var
    virAddTemp = returnVirAdd((uint64_t)pDirtyPageList, KERNBASE_ADD, 1);
    map_virt_phys_addr(virAddTemp,((uint64_t)pDirtyPageList & ADD_SCHEME));
    pDirtyPageList = (Page*)returnVirAdd((uint64_t)pDirtyPageList, KERNBASE_ADD, 0);

}

uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt)
{
    *cr3_reg = 0x0;
    *cr3_reg |= ((pwt << 3) & 0x08);
    *cr3_reg |= (pcd << 4);
    *cr3_reg |= (pml4e_add & 0xfffffffffffff000);
    return cr3_reg;
}

uint64_t getCR3(){
    uint64_t cr3 = 0;
    __asm__ __volatile__(
    "movq %%cr3, %0\n\t"
    :"=r"(cr3):);
    return cr3;
}

uint64_t returnPhyAdd(uint64_t add, short addType, short removeFlags)
{
    switch (addType){
        case KERNBASE_ADD:
        {
            if(removeFlags)
                return ((add-KERNBASE)&ADD_SCHEME);
            else
                return ((add-KERNBASE));
        }
            /*case VMAP_BASE_ADD:
            {
                if(removeFlags)
            return ((add-KERNBASE)&ADD_SCHEME);
        else
            return ((add-KERNBASE));
            }*/
    };

    kprintf("Error: address type not supported, returning same number\n");
    return add;
}

uint64_t returnVirAdd(uint64_t add, short addType, short removeFlags)
{
    switch (addType){
        case KERNBASE_ADD:
        {
            if(removeFlags)
                return (KERNBASE | (add & ADD_SCHEME));
            else
                return (add | KERNBASE);
        }
        /*case VMAP_BASE_ADD:
        {
            if(removeFlags)
                return (KERNBASE | (add & ADD_SCHEME));
            else
                return (add | KERNBASE);
        }*/
    };

    kprintf("Error: address type not supported, returning same number\n");
    return add;
}