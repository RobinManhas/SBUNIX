#include <syscalls.h>

long syscall0(const long syscall){
	long ret;
__asm__ ("movq %1,%%rax;syscall" : "=r" (ret) : "0" (syscall):"memory");
return ret; 
}


 long syscall1(const long syscall, const long arg1) 
{ 
long ret;
__asm__ ("movq %1,%%rax; movq %2,%%rdi;syscall" : "=r" (ret) : "0" (syscall), "g" (arg1):"memory");
return ret; 
}

 long syscall2(const long syscall, const long arg1, const long arg2) 
{ 
	long ret;
	__asm__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi;;syscall" : "=r" (ret):"0"(syscall), "r"(arg1), "r"(arg2):"memory" );
	return ret; 
}

 long syscall3(const long syscall,const long arg1,const long arg2,const long arg3)
{
long ret;
__asm__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi; movq %4,%%rdx;syscall" : "=r" (ret):"0"(syscall), "g"(arg1), "g"(arg2) ,"g"(arg3) :"memory" );
return ret;
}

long syscall4(const long syscall,const long arg1,const long arg2,const long arg3, const long arg4)
{
long ret;
__asm__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi; movq %4,%%rdx; movq %5,%%r10;syscall" : "=r" (ret):"0"(syscall), "g"(arg1), "g"(arg2) ,"g"(arg3),"g"(arg4) :"memory" );
return ret; 
}

