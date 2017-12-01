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
#include <sys/kmalloc.h>
#include <sys/procmgr.h>
#include <sys/common.h>
#include <sys/mm.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;
uint64_t maxPhyRegion;
extern uint64_t* pml_table;
task_struct *kernel_idle_task, *task1, *task2, *user_task;
extern void func1();
extern void func2();

void start(uint32_t *modulep, void *physbase, void *physfree)
{
    clearScreen();
    kprintf("old cr3 %x, old physfree: %x\n",getCR3(),physfree);
    maxPhyRegion = phyMemInit(modulep,physbase,&physfree);
    pageTablesInit((uint64_t) physbase, (uint64_t) physfree,(uint64_t)KERN_PHYS_BASE,(uint64_t)PTE_W_P);

    // DO NOT DELETE: Enable the below line if we want to do mapping of whole pages above physfree in
    // start itself. Currently we just map initial pml4,pdp,pd and pt, rest all pages are dynamically mapped
    // mapPhysicalRangeToVirtual(maxPhyRegion, physfree,(uint64_t)PTE_W_P);

    mapPhysicalRangeToVirtual((uint64_t)(physfree+ (4*PAGE_SIZE)), physfree,(uint64_t)PTE_W_P);

    setCR3(pml_table);

    kprintf("after cr3 reset, cr3: %x, new physfree: %x\n",getCR3(),physfree);

/*********************************************************************************************************
    * Welcome to user land, use any physical address from now on and watch qemu reboot forever.
    * Use returnVirtualAdd() and returnPhyAdd() for ease of conversion.
    * thread  : one execution path containing separate registers + stack
    * process : virtual address space + registers + stack
*********************************************************************************************************/

    init_idt();
    init_irq();
    init_keyboard();
    init_tarfs();
    syscalls_init();
    __asm__ ("sti"); //enable interrupts



    kernel_idle_task = getFreeTask();
    kernel_idle_task->name ="Idle_task";
    createKernelInitProcess(kernel_idle_task);

//    task1 = getFreeTask();
//    createKernelTask(task1,func1);

//    task2 = getFreeTask();
//    createKernelTask(task2,func2);
//
//    user_task = getFreeTask();
//    createUserProcess(user_task);

    load_elf_binary_by_name(NULL,"bin/sbush",NULL);

    kprintf("In main: init IDT and IRQ success, calling schedule\n");
    schedule();

    // do not exit start thread
//    while(1){
//        schedule();
//        __asm__ __volatile__("hlt;");
//    }


//    init_idt();
//    init_irq();
    //init_timer();

//    __asm__ ("sti");
//    init_pci();

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