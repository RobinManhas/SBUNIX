.text

.global _irq0
.global _irq1
.global isr0
.global isr14
.global syscall


_irq0:
    cli
    //pushq %rax
    //movq $32,%rax
    pushq $0
    pushq $32
    jmp irq_common_stub

_irq1:
    cli
    //pushq %rax
    //movq $33,%rax
    pushq $0
    pushq $33
    jmp irq_common_stub

isr0:
    cli
    //pushq %rax
    //movq $40,%rax  ///$40 is temp
    pushq $0
    pushq $40
    jmp irq_common_stub

isr14:
    cli
    //pushq %rax
    //movq $40,%rax  ///$40 is temp
    pushq $14
    jmp irq_common_stub


syscall:
    cli
    pushq $0
    pushq $128
    jmp irq_common_stub

irq_common_stub:


    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15



    movq %rsp, %rdi   ////
    callq  _irq_handler


    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax

    addq $16,%rsp

    sti
    iretq