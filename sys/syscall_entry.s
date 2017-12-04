.global user_rsp
.global kernel_rsp

.global syscall_entry
syscall_entry:
    movq %rsp, user_rsp     # save user stack
    movq (kernel_rsp), %rsp
    pushq (user_rsp)
    pushq %r11
    pushq %rcx
    movq %r10, %rcx    #r10 contains 5th argument syscall4
    pushq %rax
    pushq %rbx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15


    movq %rsp, %rdi
    callq syscall_handler
    jmp jmpSeq


.global forkChild
forkChild:
    xorq %rax,%rax   //clear rax register for child

.global jmpSeq
jmpSeq:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rbx
    addq $0x8, %rsp     #as rax contains return value so dont pop it
    popq %rcx
    popq %r11
    popq %rsp                       #restore user stack
    sysretq
