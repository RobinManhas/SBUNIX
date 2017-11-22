//
// Created by Harsh Gupta on 11/5/17.
//

#ifndef SBUNIX_COMON_H
#define SBUNIX_COMON_H
#include<sys/defs.h>

int read_stokes(unsigned long output,unsigned long read_length);
int writeString(char* s,uint64_t write_len);
void syscalls_init();

#endif //SBUNIX_COMON_H
