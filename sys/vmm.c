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
extern uint64_t videoOutBufAdd;

uint64_t virtualMemBase = 0;
int is_present(uint64_t val)
{
    return val & 0x01;
}

void set_present(uint64_t* val)
{
    *val |= 0x01;
}

uint64_t* set_base_addr(uint64_t* val, uint64_t base_addr)
{
    if(base_addr > KERNBASE) {
        base_addr -= KERNBASE;
    }
    *val |= (base_addr & 0xffffffffff000);
    return val;
}

uint64_t* create_table_entry(uint64_t* pTable, uint64_t cTable, uint8_t avl, uint8_t flags, uint8_t nx_bit)
{
    if(cTable > KERNBASE) {
        cTable -= KERNBASE;
    }
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
            create_table_entry(&pml4_table[i], (uint64_t)pml4_table, 0x0, 0x07, 0x00);
            kprintf("val at index 511: %x\n",pml4_table[i]);
        } else {
            create_table_entry(&pml4_table[i], 0x0, 0x0, 0x06, 0x00);
            create_table_entry(&pdp_table[i], 0x0, 0x0, 0x06, 0x00);
            create_table_entry(&pd_table[i], 0x0, 0x0, 0x06, 0x00);
            create_table_entry(&page_table[i], 0x0, 0x0, 0x06, 0x00);
        }
    }
}

void static_map_page(uint64_t vir_pg, uint64_t phys_pg)
{
    uint64_t* pml4e_entry;
    uint64_t* pdp_entry;
    uint64_t* pd_entry;

    uint16_t pml4e_offset = ((vir_pg>>39)&0x1ff);
    uint16_t pdp_offset = ((vir_pg>>30)&0x1ff);
    uint16_t pd_offset = ((vir_pg>>21)&0x1ff);
    uint16_t pt_offset = ((vir_pg>>12)&0x1ff); // TODO: should be 0xfff ?

    pml4e_entry = &pml_table[pml4e_offset];

    /* The PDPE level */
    if(is_present((uint64_t)pml4e_entry)){
        pdp_entry = &pdp_table[pdp_offset];
    } else {
        /* Create a new pdpe entry*/
        pdp_entry = create_table_entry(&pdp_table[pdp_offset], (uint64_t)&pd_table, 0x0, 0x02, 0x00); // pd is not yet created
        set_base_addr(pml4e_entry, (uint64_t)pdp_table);
        set_present(pml4e_entry);
        kprintf("new pdpe entry created pml4: %x, %x\n",pml4e_entry, *pml4e_entry);
    }

    /* The PD level */
    if(is_present((uint64_t)pdp_entry)){
        pd_entry = &pd_table[pd_offset];
    } else {
        /* Create new pde entry */
        pd_entry = create_table_entry(&pd_table[pd_offset], (uint64_t)&page_table, 0x0, 0x02, 0x00);
        set_base_addr(pdp_entry, (uint64_t)pd_table);
        set_present(pdp_entry);
        kprintf("new pd entry created, pdp: %x, %x\n",pdp_entry, *pdp_entry);
    }

    /* The PT level */

    /* Create new pte entry */
    if(!is_present((uint64_t)pd_entry)){
        create_table_entry(&page_table[pt_offset], phys_pg, 0x0, 0x03, 0x00);
        set_base_addr(pd_entry, (uint64_t)page_table);
        set_present(pd_entry);
        kprintf("new pt entry created, pd: %x, %x, ptentry: %x, %x\n",pd_entry, *pd_entry, page_table, *page_table);
    }

}

void mapFromPhyToVirRange(uint64_t phys_pg_start, uint64_t phys_pg_end, uint64_t vir_pg_start)
{

    uint64_t phys_pg = (uint64_t)phys_pg_start;
    uint64_t vir_pg = (uint64_t)vir_pg_start;
    for(; phys_pg <= (uint64_t)phys_pg_end; phys_pg = phys_pg+PAGE_SIZE, vir_pg=vir_pg+PAGE_SIZE) {
        static_map_page(vir_pg, phys_pg);
    }
    static_map_page(vir_pg, 0xB8000);
    videoOutBufAdd = vir_pg;
    virtualMemBase = vir_pg+PAGE_SIZE; /* Update the top of the virtual memory */
}

uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt)
{
    *cr3_reg = 0x0;
    *cr3_reg |= ((pwt << 3) & 0x08);
    *cr3_reg |= (pcd << 4);
    *cr3_reg |= (pml4e_add & 0xfffffffffffff000);
    return cr3_reg;
}