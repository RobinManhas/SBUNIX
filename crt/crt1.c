
//
//// long enviro;
//// long argv;
////  long argc;
//void _start(void) {
//
//	__asm__(
//     "movq 8(%rsp), %rdi;"
//     "movq 16(%rsp), %rsi;"
//     "movq 24(%rsp),%rdx;"
//     // "movq %rdi,%r10;"
//     // "addq $3,%r10;"
//     // "movq $8,%rax;"
//     // "mulq %r10;"
//     // "addq %rax,%rsp;"
//     // "movq 0(%rsp),%rdx;"
//     // "subq %rax,%rsp;"
//     "call main;");
//
//
//
//
//	// __asm__(
// //     "movq 8(%%rsp), %%rdi;"
// //     "movq 16(%%rsp), %%rsi;"
// //     "movq %%rdi,%%r10;"
// //     "addq $3,%%r10;"
// //     "movq $8,%%rax;"
// //     "mulq %%r10;"
// //     "addq %%rax,%%rsp;"
// //     "movq 0(%%rsp),%%rdx;"
// //     "subq %%rax,%%rsp;"::"g"(enviro),"g"(argv),"g"(argc):"memory");
//
//
//
//
//     // __asm__("movq %1,%%rdx;"
//     //      "movq %1,%%rsi;"
//     //      "movq %0,%%rdi"::"g"(enviro),"g"(argv),"g"(argc):"memory");
//
//    // __asm__("call main;");
//
//	while(1){
//		exit(1);
//	}
//
//}
//

#include <stdlib.h>
char **environ = 0;
__thread int errno;

void _start(void) {
	__asm__ volatile (
	"xorq %%rbp, %%rbp;"            /* ABI: zero rbp */
			"popq %%rdi;"                   /* set argc */
			"movq %%rsp, %%rsi;"            /* set argv */
			"leaq 8(%%rsi,%%rdi,8), %%rdx;" /* Set envp = argv + 8 * argc + 8 */
			"pushq %%rbp;"
			"pushq %%rbp;"
			"andq $-16, %%rsp;"             /* ABI: align stack to 16 bytes */
			"call _init_sblibc;"
			"call main;"                    /* run main program */
			"movq %%rax, %%rdi;"            /* exit with return code */
			"call exit;"
			"hlt;":::
	);
}

void _init_sblibc(int argc, char **argv, char **envp) {
	/* initialize environ which is used by getenv(3)/setenv(3) */
	environ = envp;
}


