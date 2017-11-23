//
// Created by robin manhas on 10/18/17.
//

#ifndef OS_KMALLOC_H
#define OS_KMALLOC_H
#include <sys/defs.h>

void* kmalloc(/*unsigned int size*/);


void* kmalloc_size(uint64_t size);
#endif //OS_KMALLOC_H
