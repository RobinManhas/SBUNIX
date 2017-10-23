#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/kprintf.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/idt.h>
#include <sys/util.h>
#include <sys/pci.h>
#include <sys/pmm.h>
#include <sys/vmm.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;

uint64_t uCR3;
// Ref: http://wiki.osdev.org/Setting_Up_Paging, diff that for 64 bit, 512 entries
//uint64_t pml_table[TABLE_ENTRIES_MAX] __attribute__((aligned(0x1000)));
//uint64_t pdp_table[TABLE_ENTRIES_MAX] __attribute__((aligned(0x1000)));
//uint64_t pd_table[TABLE_ENTRIES_MAX] __attribute__((aligned(0x1000)));
//uint64_t page_table[TABLE_ENTRIES_MAX] __attribute__((aligned(0x1000)));
uint64_t *pml_table;
uint64_t *pdp_table;
uint64_t *pd_table;
uint64_t *page_table;
void start(uint32_t *modulep, void *physbase, void *physfree)
{
    clearScreen();
    unsigned long cr3 = 1;
    __asm__ __volatile__(
    "movq %%cr3, %0\n\t"
    :"=r"(cr3):);
    kprintf("cr3 old: %x\n",cr3);
    phyMemInit(modulep,physbase,&physfree);
    pml_table = (uint64_t*)allocatePage();
    pdp_table = (uint64_t*)allocatePage();
    pd_table = (uint64_t*)allocatePage();
    page_table = (uint64_t*)allocatePage();
    kprintf("old pmltable: %x\n",pml_table);
    pageTablesInit(pml_table);
    kprintf("new pmltable: %x\n",pml_table);
    mapFromPhyToVirRange((uint64_t) physbase, (uint64_t) physfree, (uint64_t) (KERNBASE + physbase));
    //cr3Create(&uCR3, (uint64_t) pml_table, 0x00, 0x00);
    uCR3 = (uint64_t)pml_table;
    kprintf("new physfree: %x, cr3: %x\n",physfree,uCR3);
    //__asm__ __volatile__("movq %0, %%cr3":: "a"(uCR3));
    /* Setup the stack again. */
    //__asm__ __volatile__("movq %0, %%rbp" : :"a"(&loader_stack[0]));
    //__asm__ __volatile__("movq %0, %%rsp" : :"a"(&loader_stack[INITIAL_STACK_SIZE]));
    kprintf("done with stack init\n");

/*
    // Verification for Physical mem logic
    struct Page* p1 = allocatePage();
    kprintf("p1: %x\n",p1->uAddress);
    struct Page* p2 = allocatePage();
    kprintf("p2: %x\n",p2->uAddress);

    deallocatePage(p1);
    p1 = allocatePage();
    kprintf("after realloc, p1: %x\n",p1->uAddress);

    deallocatePage(p2);
    p2 = allocatePage();
    kprintf("after realloc, p2: %x\n",p2->uAddress);
    deallocatePage(p2);
    deallocatePage(p1);
*/
    //init_idt();
    //init_irq();
    //init_timer();
    //init_keyboard();
    //__asm__ ("sti");
    init_pci();
    //updateTimeOnScreen();

    while(1);
}

void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;

    for(temp2 = (char*)0xb8001; temp2 < (char*)0xb8000+160*25; temp2 += 2) *temp2 = 7 /* white */;
    __asm__(
    "cli;"
            "movq %%rsp, %0;"
            "movq %1, %%rsp;"
    :"=g"(loader_stack)
    :"r"(&initial_stack[INITIAL_STACK_SIZE])
    );
    init_gdt();
    start(
            (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
            (uint64_t*)&physbase,
            (uint64_t*)(uint64_t)loader_stack[4]
    );
    for(
            temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
            *temp1;
            temp1 += 1, temp2 += 2
            ) *temp2 = *temp1;
    while(1);
}