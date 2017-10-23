#include <stdlib.h>

// long enviro;
// long argv;
//  long argc;
void _start(void) {

	__asm__(
     "movq 8(%rsp), %rdi;"
     "movq 16(%rsp), %rsi;"
     "movq 24(%rsp),%rdx;"
     // "movq %rdi,%r10;"
     // "addq $3,%r10;"
     // "movq $8,%rax;"
     // "mulq %r10;"
     // "addq %rax,%rsp;"
     // "movq 0(%rsp),%rdx;"
     // "subq %rax,%rsp;"
     "call main;");




	// __asm__(
 //     "movq 8(%%rsp), %%rdi;"
 //     "movq 16(%%rsp), %%rsi;"
 //     "movq %%rdi,%%r10;"
 //     "addq $3,%%r10;"
 //     "movq $8,%%rax;"
 //     "mulq %%r10;"
 //     "addq %%rax,%%rsp;"
 //     "movq 0(%%rsp),%%rdx;"
 //     "subq %%rax,%%rsp;"::"g"(enviro),"g"(argv),"g"(argc):"memory");




     // __asm__("movq %1,%%rdx;"
     //      "movq %1,%%rsi;"
     //      "movq %0,%%rdi"::"g"(enviro),"g"(argv),"g"(argc):"memory");

    // __asm__("call main;");

	while(1){
		exit(1);
	}

}

