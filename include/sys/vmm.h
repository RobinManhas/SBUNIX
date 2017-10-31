//
// Created by robin manhas on 10/21/17.
//

#ifndef OS_PROJECT_VMM_H
#define OS_PROJECT_VMM_H

#include "sys/defs.h"

#define IDENTITY_MAP_V		0xFFFFFFFF00000000UL
#define	IDENTITY_MAP_P		0x0UL

#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Write
#define PTE_U		0x004	// User
#define ADD_SCHEME		0xFFFFFFFFFFFFF000

uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt);
uint64_t* pageTablesInit(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart);
void setIdentityPaging(uint64_t max_phy);

#endif //OS_PROJECT_VMM_H
