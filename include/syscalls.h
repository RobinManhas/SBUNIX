#ifndef _SYSCALLS_H
#define _SYSCALLS_H


#include <unistd.h>



#define NULL ((void*)0)

 long syscall0(const long syscall) ;
 long syscall1(const long syscall, const long arg1) ;
 long syscall2(const long syscall, const long arg1, const long arg2);
 long syscall3(const long syscall,const long arg1,const long arg2,const long arg3);
 long syscall4(const long syscall,const long arg1,const long arg2,const long arg3, const long arg4);

#endif