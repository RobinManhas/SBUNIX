#include <stdlib.h>
char **environ = 0;
void _start(void) {
	__asm__ volatile (
	"xorq %%rbp, %%rbp;"
			"popq %%rdi;"
			"movq %%rsp, %%rsi;"
			"leaq 8(%%rsi,%%rdi,8), %%rdx;"
			"pushq %%rbp;"
			"pushq %%rbp;"
			"andq $-16, %%rsp;"
			"call init_enviro;"
			"call main;"
			"movq %%rax, %%rdi;"
			"call exit;"
			"hlt;":::
	);
}

void init_enviro(int argc, char **argv, char **envp) {
	environ = envp;
}


