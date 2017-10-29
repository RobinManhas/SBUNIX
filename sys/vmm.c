//
// Created by robin manhas on 10/21/17.
//

#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/kprintf.h>

// Ref: http://wiki.osdev.org/Setting_Up_Paging, diff that for 64 bit, 512 entries
extern uint64_t *pml_table;
extern uint64_t *pdp_table;
extern uint64_t *pd_table;
extern uint64_t *page_table;
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

int checkPresentBit(uint64_t val)
{
    return val & 0x01;
}

void setPresentBit(uint64_t *val)
{
    *val |= 0x01;
}

uint64_t* setBaseAddress(uint64_t *val, uint64_t base_addr)
{
    if(base_addr > KERNBASE)
        base_addr -= KERNBASE;

    *val |= (base_addr & 0xffffffffff000);
    return val;
}

uint64_t* tableEntryCreate(uint64_t *pTable, uint64_t cTable, uint8_t avl, uint8_t flags, uint8_t nx_bit)
{
    if(cTable > KERNBASE)
        cTable -= KERNBASE;

    *pTable = 0x0;
    *pTable |= ((uint64_t)nx_bit << 63);
    *pTable |= (cTable & 0xfffffffffffff000);
    *pTable |= (avl << 9);
    *pTable |= (flags & 0x7F);
    return pTable;
}

// RM: Initialize table values with 0's
void pageTablesInit(uint64_t *pml4_table)
{
    for(int i=0; i<TABLE_ENTRIES_MAX; i++){
        if (i==510)
        {
            /* Self-Referencing PML for phy mem add.. https://piazza.com/class/j6a5xl76dlt1zn?cid=334*/
            //kprintf("val at index 510 before: %x, pml4 val: %x\n",pml4_table[i],pml4_table);
            tableEntryCreate(&pml4_table[i], (uint64_t) pml4_table, 0x0, 0x07, 0x00); // set User, R/Write, Present (in phy mem)
            //kprintf("val at index 510: %x\n",pml4_table[i]);
        }
        else
        {
            tableEntryCreate(&pml4_table[i], 0x0, 0x0, 0x06, 0x00); // set User, R/Write
        }
        tableEntryCreate(&pdp_table[i], 0x0, 0x0, 0x06, 0x00);
        tableEntryCreate(&pd_table[i], 0x0, 0x0, 0x06, 0x00);
        tableEntryCreate(&page_table[i], 0x0, 0x0, 0x06, 0x00);
    }
}

// func maps the physical kernel space to virtual address space beginning KERNBASE
void staticPageMapper(uint64_t virtualPage, uint64_t phyPage)
{
    uint64_t* pmlEntryAddress;
    uint64_t* pdpEntryAdd;
    uint64_t* pdEntryAdd;

    uint16_t pml4Offset = ((virtualPage>>39)&0x1ff);
    uint16_t pdpOffset = ((virtualPage>>30)&0x1ff);
    uint16_t pdOffset = ((virtualPage>>21)&0x1ff);
    uint16_t ptOffset = ((virtualPage>>12)&0x1ff); // RM: 1ff as bit 9-11 unused

    /* RM Important Info, Do not delete:
    Since kernbase currently sits at 0xffffffff80200000, statically mapping
    this address yields the pml4 offset to be 511(max), therefore if we really
     need to setup recursive mapping as per http://wiki.osdev.org/Page_Tables,
     we need to set the same at index 510
     For kern base address 0xffffffff80200000, we have offsets:
     PML: 0xFF = 511
     PDP: 0x1FE = 510
     PD : 0x01 = 1
     PT : 0x01 = 0
     */
    kprintf("pmof: %d, pdpof: %d, pdof: %d,ptof: %d, vir: %x, ph: %x\n",
            pml4Offset, pdpOffset,pdOffset, ptOffset,virtualPage, phyPage);

    pmlEntryAddress = &pml_table[pml4Offset];
    // PDP
    if(checkPresentBit((uint64_t) pmlEntryAddress))
    {
        pdpEntryAdd = &pdp_table[pdpOffset];
    }
    else
    {

        pdpEntryAdd = tableEntryCreate(&pdp_table[pdpOffset], (uint64_t) &pd_table, 0x0, 0x02, 0x00); // set only R/Write flags
        setBaseAddress(pmlEntryAddress, (uint64_t) pdp_table);
        setPresentBit(pmlEntryAddress);
    }

    // PD
    if(checkPresentBit((uint64_t) pdpEntryAdd))
    {
        pdEntryAdd = &pd_table[pdOffset];
    }
    else
    {
        /* Create new pde entry */
        pdEntryAdd = tableEntryCreate(&pd_table[pdOffset], (uint64_t) &page_table, 0x0, 0x02, 0x00);
        setBaseAddress(pdpEntryAdd, (uint64_t) pd_table);
        setPresentBit(pdpEntryAdd);
    }

    // PT
    if(!checkPresentBit((uint64_t) pdEntryAdd)){
        tableEntryCreate(&page_table[ptOffset], phyPage, 0x0, 0x03, 0x00);
        setBaseAddress(pdEntryAdd, (uint64_t) page_table);
        setPresentBit(pdEntryAdd);

    }
    kprintf("new pml:%x, pdp: %x, pd: %x, pt: %x, phyAdd: %x\n",
            pmlEntryAddress,pml_table[pml4Offset],pdp_table[pdpOffset],pd_table[pdOffset],page_table[ptOffset]);
}

void mapFromPhyToVirRange(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart)
{

    uint64_t phyPage = (uint64_t)phyPageStart;
    uint64_t virPage = (uint64_t)virPageStart;
    for(; phyPage <= (uint64_t)phyPageEnd; phyPage = phyPage+PAGE_SIZE, virPage=virPage+PAGE_SIZE) {
        //kprintf("Mapping phy: %x <--> vir: %x\n",phyPage, virPage);
        staticPageMapper(virPage, phyPage);
    }
    staticPageMapper(virPage, 0xB8000);
    videoOutBufAdd = virPage;
    virtualMemBase = virPage+PAGE_SIZE;
}