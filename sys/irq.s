.text

.global _irq0
.global _irq1
.global isr0
.global isr14
.global isr13

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
    pushq $14
    jmp irq_common_stub

isr13:
    cli
    pushq $13
    jmp irq_common_stub


syscall:
    cli
    pushq $0
    pushq $128
    jmp irq_common_stub

irq_common_stub:


    pushq %r11
    pushq %rcx
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
    callq  _irq_handler


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
    popq %rax
    popq %rcx
    popq %r11

    addq $16,%rsp

    sti
    iretq