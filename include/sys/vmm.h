//
// Created by robin manhas on 10/21/17.
//

#ifndef OS_PROJECT_VMM_H
#define OS_PROJECT_VMM_H

#include "sys/defs.h"

void pageTablesInit(uint64_t *pml4_entries);
void staticPageMapper(uint64_t virtualPage, uint64_t phyPage);
void mapFromPhyToVirRange(uint64_t phyPageStart, uint64_t phyPageEnd, uint64_t virPageStart);
uint64_t * cr3Create(uint64_t *cr3_reg, uint64_t pml4e_add, int pcd, int pwt);

#endif //OS_PROJECT_VMM_H
