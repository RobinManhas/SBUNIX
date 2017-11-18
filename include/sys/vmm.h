//
// Created by robin manhas on 10/21/17.
//

#ifndef OS_PROJECT_VMM_H
#define OS_PROJECT_VMM_H

#include "sys/defs.h"

// KERNBASE acts as base address for all kernel related code only.
#define KERN_PHYS_BASE 0xffffffff80200000UL

#define	IDENTITY_MAP_P		0x0UL

#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Write
#define PTE_U		0x004	// User
#define PTE_W_P     0x007  // premission for Supervisor, write and present
#define PTE_U_W_P   0x007    // premission for User, write and present

#define ADDRESS_SCHEME		0xFFFFFFFFFFFFF000
#define KERNBASE_OFFSET 0  // address that was/needs offset by KERNBASE
#define VMAP_BASE_ADD 1 // address that was/needs offset by VMAP_BASE

uint64_t getCR3();
void setCR3(uint64_t* pmlAdd);
uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt);
uint64_t* pageTablesInit(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart, uint64_t flags);
void mapPhysicalRangeToVirtual(uint64_t max_phy, void *physfree, uint64_t flags);
uint64_t returnPhyAdd(uint64_t add, short addType, short removeFlags);
uint64_t returnVirAdd(uint64_t add, short addType, short removeFlags);
void map_virt_phys_addr(uint64_t vaddr, uint64_t paddr, uint64_t flags);
uint64_t* getKernelPML4();

#endif //OS_PROJECT_VMM_H
