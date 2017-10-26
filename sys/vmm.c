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
        if (i==511) {
            /* Self-Referencing PML for phy mem add.. https://piazza.com/class/j6a5xl76dlt1zn?cid=334*/
            kprintf("val at index 511 before: %x, pml4 val: %x\n",pml4_table[i],pml4_table);
            tableEntryCreate(&pml4_table[i], (uint64_t) pml4_table, 0x0, 0x07, 0x00);
            kprintf("val at index 511: %x\n",pml4_table[i]);
        } else {
            tableEntryCreate(&pml4_table[i], 0x0, 0x0, 0x06, 0x00);
            tableEntryCreate(&pdp_table[i], 0x0, 0x0, 0x06, 0x00);
            tableEntryCreate(&pd_table[i], 0x0, 0x0, 0x06, 0x00);
            tableEntryCreate(&page_table[i], 0x0, 0x0, 0x06, 0x00);
        }
    }
}

// func maps the physical kernel space to virtual address space beginning KERNBASE
void staticPageMapper(uint64_t vir_pg, uint64_t phys_pg)
{
    uint64_t* pml4eTableAdd;
    uint64_t* pdpTableAdd;
    uint64_t* pdTableAdd;

    uint16_t pml4e_offset = ((vir_pg>>39)&0x1ff);
    uint16_t pdp_offset = ((vir_pg>>30)&0x1ff);
    uint16_t pd_offset = ((vir_pg>>21)&0x1ff);
    uint16_t pt_offset = ((vir_pg>>12)&0x1ff);

    kprintf("pmloff: %d, pdpoff: %d, pdoff: %d,ptoff: %d, vir: %x, phy: %x\n",
            pml4e_offset, pdp_offset,pd_offset, pt_offset,vir_pg, phys_pg);

    pml4eTableAdd = &pml_table[pml4e_offset];
    kprintf("new pml entry %x\n",pml_table[pml4e_offset]);
    // PDP
    if(checkPresentBit((uint64_t) pml4eTableAdd)){
        pdpTableAdd = &pdp_table[pdp_offset];
    } else {

        pdpTableAdd = tableEntryCreate(&pdp_table[pdp_offset], (uint64_t) &pd_table, 0x0, 0x02, 0x00);
        setBaseAddress(pml4eTableAdd, (uint64_t) pdp_table);
        setPresentBit(pml4eTableAdd);
        kprintf("new pdp entry %x\n",pdp_table[pdp_offset]);
    }

    // PD
    if(checkPresentBit((uint64_t) pdpTableAdd)){
        pdTableAdd = &pd_table[pd_offset];
    } else {
        /* Create new pde entry */
        pdTableAdd = tableEntryCreate(&pd_table[pd_offset], (uint64_t) &page_table, 0x0, 0x02, 0x00);
        setBaseAddress(pdpTableAdd, (uint64_t) pd_table);
        setPresentBit(pdpTableAdd);
        kprintf("new pd entry %x\n",pd_table[pd_offset]);
    }

    // PT
    if(!checkPresentBit((uint64_t) pdTableAdd)){
        tableEntryCreate(&page_table[pt_offset], phys_pg, 0x0, 0x03, 0x00);
        setBaseAddress(pdTableAdd, (uint64_t) page_table);
        setPresentBit(pdTableAdd);
        kprintf("new pt entry %x\n",page_table[pt_offset]);
    }

}

void mapFromPhyToVirRange(uint64_t phys_pg_start, uint64_t phys_pg_end, uint64_t vir_pg_start)
{

    uint64_t phys_pg = (uint64_t)phys_pg_start;
    uint64_t vir_pg = (uint64_t)vir_pg_start;
    for(; phys_pg <= (uint64_t)phys_pg_end; phys_pg = phys_pg+PAGE_SIZE, vir_pg=vir_pg+PAGE_SIZE) {
        kprintf("static mapping phy: %x to vir: %x\n",phys_pg, vir_pg);
        staticPageMapper(vir_pg, phys_pg);
    }
    staticPageMapper(vir_pg, 0xB8000);
    videoOutBufAdd = vir_pg;
    virtualMemBase = vir_pg+PAGE_SIZE; /* Update the top of the virtual memory */
}