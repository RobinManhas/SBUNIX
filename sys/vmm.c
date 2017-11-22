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

uint64_t* getKernelPML4(){
    return pml_table;
}

// RM: Init table for kernel, must add 0x003 permissions for kernel pages
uint64_t* pageTablesInit(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart, uint64_t flags)
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
    value |= (flags);
    pml_table[pml4Off] = value;

    pd = (uint64_t*)allocatePage();
    value = (uint64_t)pd;
    value |= (flags);
    pdp[pdpOff] = value;

    pt = (uint64_t*)allocatePage();
    value = (uint64_t)pt;
    value |= (flags);
    pd[pdOff] = value;

    for(;phyPageStart<phyPageEnd; phyPageStart += 0x1000, virPageStart += 0x1000)
    {

        uint16_t ptOff = ((virPageStart>>12)&0x1ff);
        uint64_t entry = phyPageStart;
        entry |= (flags);
        pt[ptOff] = entry;

       // kprintf("pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff]);
    }

    //kprintf("pm:%x,pdp:%x,pp:%x,pt:%x\n",pml_table,pdp,pd,pt);

    return pml_table;
}

void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    uint16_t pml4Off = ((vaddr>>39)&0x1ff);
    uint16_t pdpOff = ((vaddr>>30)&0x1ff);
    uint16_t pdOff = ((vaddr>>21)&0x1ff);
    uint16_t ptOff = ((vaddr>>12)&0x1ff);

    //kprintf("vadd: %x, padd: %x, pml: %x, pmloff: %d\n",vaddr,paddr, pml_table,pml4Off);
    uint64_t pml4_entry = pml_table[pml4Off];
    if(pml4_entry & PTE_P){
        pdp = (uint64_t*)(pml4_entry & ADDRESS_SCHEME);
    }
    else
    {
        pdp = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pdp,KERNBASE_OFFSET,1),((uint64_t)pdp & ADDRESS_SCHEME),(uint64_t)PTE_W_P);
        value = (uint64_t)pdp;
        value |= (flags);
        pml_table[pml4Off] = value;
    }

    // these checks convert physical address entries to virtual if the mapping scheme has been changed to virtual mode.
    if((uint64_t)pml_table > KERNBASE && (uint64_t)pdp < KERNBASE){
        pdp = (uint64_t*)returnVirAdd((uint64_t)pdp,KERNBASE_OFFSET,0);
    }

    uint64_t pdpt_entry = pdp[pdpOff];
    if(pdpt_entry & PTE_P){
        pd = (uint64_t*)(pdpt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pd = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pd,KERNBASE_OFFSET,1),((uint64_t)pd & ADDRESS_SCHEME),(uint64_t)PTE_W_P);
        value = (uint64_t)pd;
        value |= (flags);
        pdp[pdpOff] = value;
    }
    if((uint64_t)pml_table > KERNBASE && (uint64_t)pd < KERNBASE){
        pd = (uint64_t*)returnVirAdd((uint64_t)pd,KERNBASE_OFFSET,0);
    }

    uint64_t pdt_entry = pd[pdOff];
    if(pdt_entry & PTE_P){
        pt = (uint64_t*)(pdt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pt = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pt,KERNBASE_OFFSET,1),((uint64_t)pt & ADDRESS_SCHEME),(uint64_t)PTE_W_P);
        value = (uint64_t)pt;
        value |= (flags);
        pd[pdOff] = value;
    }

    if((uint64_t)pml_table > KERNBASE && (uint64_t)pt < KERNBASE){
        pt = (uint64_t*)returnVirAdd((uint64_t)pt,KERNBASE_OFFSET,0);
    }

    value = paddr;
    value |= (flags);
    pt[ptOff] = value;

    //kprintf("IP pm:%x,pdp:%x,pp:%x,pt:%x,pml:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff],pml_table);
    return;
}

void mapPhysicalRangeToVirtual(uint64_t max_phy, void *physfree, uint64_t flags)
{
    //kprintf("max: %x, physfree: %x\n",max_phy,physfree);
    uint64_t pbaseAdd = ((uint64_t)physfree) & ADDRESS_SCHEME;
    uint64_t vaddr = (KERNBASE | pbaseAdd);
    uint64_t paddr =  pbaseAdd;
    uint64_t max_phys = max_phy;
    for(; paddr <= max_phys; paddr += PAGE_SIZE, vaddr += PAGE_SIZE){
        map_virt_phys_addr(vaddr, paddr,flags);
    }

    kprintf("page mapping complete, reset vid ptr, free: %x\n",pFreeList);
    map_virt_phys_addr((uint64_t)0xffffffff800b8000UL, 0xb8000UL,flags);
    videoOutBufAdd = (uint64_t)0xffffffff800b8000UL;

    // update vmem top ptr
    virtualMemBase = vaddr;

//    // RM: debug code for page table entries, **DO NOT DELETE**
//    uint64_t viradd = (KERNBASE | ((uint64_t) pFreeList & ADDRESS_SCHEME));
//    uint16_t pml4Off = ((viradd >> 39)&0x1ff);
//    uint16_t pdpOff = ((viradd >> 30)&0x1ff);
//    uint16_t pdOff = ((viradd >> 21)&0x1ff);
//    uint16_t ptOff = ((viradd >> 12)&0x1ff);
//    uint64_t *pdpval = (uint64_t*)(pml_table[pml4Off] & ADDRESS_SCHEME);
//    uint64_t *pdval = (uint64_t*)(pdpval[pdpOff] & ADDRESS_SCHEME);
//    uint64_t *ptval = (uint64_t*)(pdval[pdOff] & ADDRESS_SCHEME);
//    kprintf("freelist pml:%x,pdp:%x,pp:%x,pt:%x\n",pml_table[pml4Off],pdpval[pdpOff],pdval[pdOff],ptval[ptOff]);


    // update pml global var
    pml_table = (uint64_t*)(KERNBASE | (uint64_t)pml_table);

    // update freelist global var
    uint64_t virAddTemp = returnVirAdd((uint64_t)pFreeList, KERNBASE_OFFSET, 1);
    map_virt_phys_addr(virAddTemp,((uint64_t)pFreeList & ADDRESS_SCHEME),flags);
    pFreeList = (Page*)returnVirAdd((uint64_t)pFreeList, KERNBASE_OFFSET, 0);

    // update dirty list global var
    virAddTemp = returnVirAdd((uint64_t)pDirtyPageList, KERNBASE_OFFSET, 1);
    map_virt_phys_addr(virAddTemp,((uint64_t)pDirtyPageList & ADDRESS_SCHEME),flags);
    pDirtyPageList = (Page*)returnVirAdd((uint64_t)pDirtyPageList, KERNBASE_OFFSET, 0);

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

void setCR3(uint64_t* pmlAdd){
    uint64_t uCR3;
    cr3Create(&uCR3,((uint64_t)pmlAdd & KERNMASK), 0x00, 0x00);
    //kprintf("pml4: %x, value: %x\n",pmlAdd,uCR3);
    __asm__ __volatile__("movq %0, %%cr3":: "r"(uCR3));
}

uint64_t returnPhyAdd(uint64_t add, short addType, short removeFlags)
{
    switch (addType){
        case KERNBASE_OFFSET:
        {
            if(add >= KERNBASE)
            {
                if(removeFlags)
                    return ((add-KERNBASE)&ADDRESS_SCHEME);
                else
                    return ((add-KERNBASE));
            }
            else // not adding kernbase offset
            {
                kprintf("Error: Address passed already in physical add range\n");
                if(removeFlags)
                    return (add & ADDRESS_SCHEME);
                else
                    return add;
            }
        }
        case VMAP_BASE_ADD:
        {
                if(add >= VIRBASE)
            {
                if(removeFlags)
                    return ((add-VIRBASE)&ADDRESS_SCHEME);
                else
                    return ((add-VIRBASE));
            }
            else // not adding VIRBASE offset
            {
                kprintf("Error: Address passed already in physical add range\n");
                if(removeFlags)
                    return (add & ADDRESS_SCHEME);
                else
                    return add;
            }
        }
    };

    kprintf("Error: address type not supported, returning same number\n");
    return add;
}

uint64_t returnVirAdd(uint64_t add, short addType, short removeFlags)
{
    switch (addType){
        case KERNBASE_OFFSET:
        {
            if(add < KERNBASE)
            {
                if(removeFlags)
                    return (KERNBASE | (add & ADDRESS_SCHEME));
                else
                    return (add | KERNBASE);
            }
            else // not adding kernbase offset
            {
                kprintf("Error: Address passed already in virtual add range\n");
                if(removeFlags)
                    return (add & ADDRESS_SCHEME);
                else
                    return add;
            }
        }
        case VMAP_BASE_ADD:
        {
            if(add < VIRBASE)
            {
                if(removeFlags)
                    return (VIRBASE | (add & ADDRESS_SCHEME));
                else
                    return (add | VIRBASE);
            }
            else // not adding VIRBASE offset
            {
                kprintf("Error: Address passed already in virtual add range\n");
                if(removeFlags)
                    return (add & ADDRESS_SCHEME);
                else
                    return add;
            }
        }
    };

    kprintf("Error: address type not supported, returning same number\n");
    if(removeFlags)
        return (add & ADDRESS_SCHEME);
    else
        return add;
}

uint64_t get_new_cr3(int is_user_task){

    uint64_t phyPage = allocatePage();
    short  addType =KERNBASE_OFFSET;
    uint64_t flags = PTE_W_P;

    if(is_user_task ==1){
        addType = VMAP_BASE_ADD;
        flags= PTE_U_W_P;
    }
    uint64_t viPage = returnVirAdd(phyPage,addType,1);
    map_virt_phys_addr(viPage,((uint64_t)phyPage & ADDRESS_SCHEME),flags);

    return viPage;


}

uint64_t* get_pt_entry( uint64_t vir_addr, int isUser){
    uint64_t* cr3 = (uint64_t*)getCR3();
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t base = KERNBASE;
    uint64_t base_offset = KERNBASE_OFFSET;
    if(isUser){
        base = VIRBASE;
        base_offset = VMAP_BASE_ADD;
    }
    uint16_t pml4Off = ((vir_addr>>39)&0x1ff);
    uint16_t pdpOff = ((vir_addr>>30)&0x1ff);
    uint16_t pdOff = ((vir_addr>>21)&0x1ff);
    uint16_t ptOff = ((vir_addr>>12)&0x1ff);

    uint64_t pml4_entry = cr3[pml4Off];
    if(pml4_entry & PTE_P) {
        pdp = (uint64_t *) (pml4_entry & ADDRESS_SCHEME);
        // these checks convert physical address entries to virtual if the mapping scheme has been changed to virtual mode.
        if ((uint64_t) pml_table > base && (uint64_t) pdp < base) {
            pdp = (uint64_t *) returnVirAdd((uint64_t) pdp, base_offset, 0);
        }
        uint64_t pdpt_entry = pdp[pdpOff];
        if (pdpt_entry & PTE_P) {
            pd = (uint64_t *) (pdpt_entry & ADDRESS_SCHEME);
            if ((uint64_t) pml_table > base && (uint64_t) pd < base) {
                pd = (uint64_t *) returnVirAdd((uint64_t) pd, base_offset, 0);
            }
            uint64_t pdt_entry = pd[pdOff];
            if (pdt_entry & PTE_P) {
                pt = (uint64_t *) (pdt_entry & ADDRESS_SCHEME);
                if ((uint64_t) pml_table > base && (uint64_t) pt < base) {
                    pt = (uint64_t *)returnVirAdd((uint64_t) pt, base_offset, 0);
                }
                return &pt[ptOff];
            }
        }
    }
    return NULL;

}

//need to check flags in allocate page next line
void map_virt_phys_addr_cr3(uint64_t vaddr, uint64_t paddr, uint64_t flags,int isUser)
{
    uint64_t* cr3 = (uint64_t *)getCR3();
    uint64_t *pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    uint16_t pml4Off = ((vaddr>>39)&0x1ff);
    uint16_t pdpOff = ((vaddr>>30)&0x1ff);
    uint16_t pdOff = ((vaddr>>21)&0x1ff);
    uint16_t ptOff = ((vaddr>>12)&0x1ff);
    uint64_t base = KERNBASE;
    uint64_t base_offset = KERNBASE_OFFSET;
    if(isUser){
        base = VIRBASE;
        base_offset = VMAP_BASE_ADD;
    }

    //kprintf("vadd: %x, padd: %x, pml: %x, pmloff: %d\n",vaddr,paddr, pml_table,pml4Off);
    uint64_t pml4_entry = cr3[pml4Off];
    if(pml4_entry & PTE_P){
        pdp = (uint64_t*)(pml4_entry & ADDRESS_SCHEME);
    }
    else
    {
        pdp = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pdp,base_offset,1),((uint64_t)pdp & ADDRESS_SCHEME),flags);
        value = (uint64_t)pdp;
        value |= (flags);
        pml_table[pml4Off] = value;
    }

    // these checks convert physical address entries to virtual if the mapping scheme has been changed to virtual mode.
    if((uint64_t)cr3 > base && (uint64_t)pdp < base){
        pdp = (uint64_t*)returnVirAdd((uint64_t)pdp,base_offset,0);
    }
    uint64_t pdpt_entry = pdp[pdpOff];
    if(pdpt_entry & PTE_P){
        pd = (uint64_t*)(pdpt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pd = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pd,base_offset,1),((uint64_t)pd & ADDRESS_SCHEME),(uint64_t)flags);
        value = (uint64_t)pd;
        value |= (flags);
        pdp[pdpOff] = value;
    }
    if((uint64_t)pml_table > base && (uint64_t)pd < base){
        pd = (uint64_t*)returnVirAdd((uint64_t)pd,base_offset,0);
    }
    uint64_t pdt_entry = pd[pdOff];
    if(pdt_entry & PTE_P){
        pt = (uint64_t*)(pdt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pt = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pt,base_offset,1),((uint64_t)pt & ADDRESS_SCHEME),(uint64_t)flags);
        value = (uint64_t)pt;
        value |= (flags);
        pd[pdOff] = value;
    }

    if((uint64_t)pml_table > base && (uint64_t)pt < base){
        pt = (uint64_t*)returnVirAdd((uint64_t)pt,base_offset,0);
    }

    value = paddr;
    value |= (flags);
    pt[ptOff] = value;

    //kprintf("IP pm:%x,pdp:%x,pp:%x,pt:%x,pml:%x\n",pml_table[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff],pml_table);
    return;
}

void map_user_virt_phys_addr(uint64_t vaddr, uint64_t paddr, uint64_t** pml_ptr)
{
    uint64_t *pml=NULL,*pdp=NULL,*pd=NULL,*pt=NULL;
    uint64_t value;
    uint16_t pml4Off = ((vaddr>>39)&0x1ff);
    uint16_t pdpOff = ((vaddr>>30)&0x1ff);
    uint16_t pdOff = ((vaddr>>21)&0x1ff);
    uint16_t ptOff = ((vaddr>>12)&0x1ff);

    //kprintf("pmoff: %d, pdpof: %d, pdof: %d, ptof: %d\n",pml4Off,pdpOff, pdOff,ptOff);
    pml = *pml_ptr;
    uint64_t pml4_entry = pml[pml4Off];
    if(pml4_entry & PTE_P){
        pdp = (uint64_t*)(pml4_entry & ADDRESS_SCHEME);
    }
    else
    {
        pdp = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pdp,KERNBASE_OFFSET,1),((uint64_t)pdp & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
        value = (uint64_t)pdp;
        value |= (PTE_U_W_P);
        pml[pml4Off] = value;
    }

    // these checks convert physical address entries to virtual if the mapping scheme has been changed to virtual mode.
    if((uint64_t)pml > KERNBASE && (uint64_t)pdp < KERNBASE){
        pdp = (uint64_t*)returnVirAdd((uint64_t)pdp,KERNBASE_OFFSET,0);
    }

    uint64_t pdpt_entry = pdp[pdpOff];
    if(pdpt_entry & PTE_P){
        pd = (uint64_t*)(pdpt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pd = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pd,KERNBASE_OFFSET,1),((uint64_t)pd & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
        value = (uint64_t)pd;
        value |= (PTE_U_W_P);
        pdp[pdpOff] = value;
    }
    if((uint64_t)pml > KERNBASE && (uint64_t)pd < KERNBASE){
        pd = (uint64_t*)returnVirAdd((uint64_t)pd,KERNBASE_OFFSET,0);
    }

    uint64_t pdt_entry = pd[pdOff];
    if(pdt_entry & PTE_P){
        pt = (uint64_t*)(pdt_entry & ADDRESS_SCHEME);
    }
    else
    {
        pt = (uint64_t*)allocatePage();
        map_virt_phys_addr(returnVirAdd((uint64_t)pt,KERNBASE_OFFSET,1),((uint64_t)pt & ADDRESS_SCHEME),(uint64_t)PTE_U_W_P);
        value = (uint64_t)pt;
        value |= (PTE_U_W_P);
        pd[pdOff] = value;
    }

    if((uint64_t)pml > KERNBASE && (uint64_t)pt < KERNBASE){
        pt = (uint64_t*)returnVirAdd((uint64_t)pt,KERNBASE_OFFSET,0);
    }

    value = paddr;
    value |= (PTE_U_W_P);
    pt[ptOff] = value;

    //kprintf("U pm:%x,pdp:%x,pp:%x,pt:%x,pml:%x\n",pml[pml4Off],pdp[pdpOff],pd[pdOff],pt[ptOff],pml);
    return;
}