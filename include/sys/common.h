//
// Created by Harsh Gupta on 11/5/17.
//

#ifndef SBUNIX_COMON_H
#define SBUNIX_COMON_H
#include<sys/defs.h>

//#define DEBUG_LOGS_ENABLE
//#define ERROR_LOGS_ENABLE

#define SIGINT    2
#define SIGKILL   9
#define SIGSEGV   11
#define SIGTERM   15

void syscalls_init();

#endif //SBUNIX_COMON_H
