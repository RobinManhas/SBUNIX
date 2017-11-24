.text

.global _irq0
.global _irq1
.global isr0
.global isr14
.global ISR_HANDLER_0
.global ISR_HANDLER_1

.global ISR_HANDLER_2

.global ISR_HANDLER_3

.global ISR_HANDLER_4

.global ISR_HANDLER_5
.global ISR_HANDLER_6

.global ISR_HANDLER_7

.global ISR_HANDLER_8

.global ISR_HANDLER_9
.global ISR_HANDLER_10

.global ISR_HANDLER_11

.global ISR_HANDLER_12
.global ISR_HANDLER_13
.global ISR_HANDLER_16
.global ISR_HANDLER_17

.global ISR_HANDLER_18

.global ISR_HANDLER_19
.global ISR_HANDLER_20
.global ISR_HANDLER_21
.global ISR_HANDLER_22
.global ISR_HANDLER_23

.global ISR_HANDLER_24

.global ISR_HANDLER_25
.global ISR_HANDLER_26
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
    pushq $0
    pushq $14
    jmp irq_common_stub

ISR_HANDLER_0:
    cli
    pushq $0
    pushq $0
    jmp irq_common_stub

ISR_HANDLER_1:
    cli
    pushq $0
    pushq $1
    jmp irq_common_stub
ISR_HANDLER_2:
   cli
   pushq $0
   pushq $2
   jmp irq_common_stub


ISR_HANDLER_3:
    cli
    pushq $0
    pushq $3
    jmp irq_common_stub

ISR_HANDLER_4:
    cli
    pushq $0
    pushq $4
    jmp irq_common_stub

ISR_HANDLER_5:
    cli
    pushq $0
    pushq $5
    jmp irq_common_stub
ISR_HANDLER_6:
   cli
   pushq $0
   pushq $6
   jmp irq_common_stub
ISR_HANDLER_7:
    cli
    pushq $0
    pushq $7
    jmp irq_common_stub
ISR_HANDLER_8:
    cli
    pushq $0
    pushq $8
    jmp irq_common_stub

ISR_HANDLER_9:
    cli
    pushq $0
    pushq $9
    jmp irq_common_stub
ISR_HANDLER_10:
    cli
    pushq $0
    pushq $10
    jmp irq_common_stub

ISR_HANDLER_11:
    cli
    pushq $0
    pushq $11
    jmp irq_common_stub
ISR_HANDLER_12:
    cli
    pushq $0
    pushq $12
    jmp irq_common_stub

ISR_HANDLER_13:
    cli
    pushq $13
    jmp irq_common_stub
ISR_HANDLER_19:
    cli
    pushq $0
    pushq $19
    jmp irq_common_stub

ISR_HANDLER_16:
    cli
    pushq $0
    pushq $16
    jmp irq_common_stub
ISR_HANDLER_17:
    cli
    pushq $0
    pushq $17
    jmp irq_common_stub

ISR_HANDLER_18:
    cli
    pushq $0
    pushq $18
    jmp irq_common_stub
ISR_HANDLER_20:
   cli
   pushq $0
   pushq $20
   jmp irq_common_stub

ISR_HANDLER_21:
   cli
   pushq $0
   pushq $21
   jmp irq_common_stub

ISR_HANDLER_22:
   cli
   pushq $0
   pushq $22
   jmp irq_common_stub

ISR_HANDLER_23:
   cli
   pushq $0
   pushq $23
   jmp irq_common_stub

ISR_HANDLER_24:
   cli
   pushq $0
   pushq $24
   jmp irq_common_stub
ISR_HANDLER_25:
   cli
   pushq $0
   pushq $25
   jmp irq_common_stub

ISR_HANDLER_26:
   cli
   pushq $0
   pushq $26
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



    movq %rsp, %rdi   ////
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