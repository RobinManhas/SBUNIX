#include <unistd.h>
#include <sys/idt.h>
#include <sys/procmgr.h>
#include <sys/common.h>
#include<sys/kprintf.h>
#include <sys/mm.h>
#include <sys/pmm.h>

#define MSR_EFER 0xc0000080		/* extended feature register */
#define MSR_STAR 0xc0000081		/* legacy mode SYSCALL target */
#define MSR_LSTAR 0xc0000082 		/* long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084	/* EFLAGS mask for syscall */



extern void syscall_entry();
uint64_t user_rsp;
uint64_t kernel_rsp;
task_struct* CURRENT_TASK;


/**
 * Write a 64-bit value to a MSR. The A constraint stands for concatenation
 * of registers EAX and EDX.
 */
void wrmsr(uint64_t id, uint64_t val) {
    uint32_t msr_lo, msr_hi;
    msr_lo = (uint32_t)val;
    msr_hi = (uint32_t)(val >> 32);
    __asm__ __volatile__ ("wrmsr" : : "a"(msr_lo), "d"(msr_hi), "c"(id));
}

uint64_t rdmsr(uint64_t id) {
    uint32_t msr_lo, msr_hi;
    __asm__ __volatile__ ( "rdmsr" : "=a" (msr_lo), "=d" (msr_hi) : "c" (id));
    return (uint64_t) msr_hi <<32 | (uint64_t) msr_lo;
}

void syscalls_init() {
    kprintf("Inside syscall init\n");
    uint64_t efer = rdmsr(MSR_EFER)|0x1;
    wrmsr(MSR_EFER, efer);

   // wrmsr(MSR_STAR, (uint64_t)0x8 << 32 | (uint64_t)0x23 << 48);
    //uint64_t val = ((uint64_t)0x23 << 56 | (uint64_t)0x1b << 48 | (uint64_t)0x10 << 40 | (uint64_t)0x8 << 32);
    uint64_t val = ((uint64_t)0x1b << 48 |(uint64_t)0x8 << 32);
    wrmsr(MSR_STAR,val);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);
    wrmsr(MSR_SYSCALL_MASK, 1<<9);
}



uint64_t sread(uint64_t fdn, uint64_t addr,uint64_t len) {
    uint64_t read_length = -1;
    if(fdn<MAX_FD)
    read_length = CURRENT_TASK->fd[fdn]->fileOps->read_file(fdn,addr,len);
    //read_length = read_file(fdn,addr,len);
    return read_length;
}

uint64_t swrite(uint64_t fdn, uint64_t addr,uint64_t len){
//    if (fdn == 1 || fdn == 2) {
//        len = writeString((char *)addr, len);
//        return len;
//    }
    uint64_t len_write = -1;
    if(fdn<MAX_FD)
    len_write = CURRENT_TASK->fd[fdn]->fileOps->write_file((char *)addr,len);
    return len_write;
}

void skill(/* kills the current active process */){
    killActiveProcess();
}

//uint64_t sclose(uint64_t fdn){
//    currentTask->fd[fdn]=NULL;
//    //need to check inode for fs extra 10 points??
//    return 1;
//}
//
//uint64_t sgetpid(){
//    return currentTask->pid;
//}
//
//uint64_t sdup2(uint64_t oldfd , uint64_t newfd){
//    if (newfd == oldfd)
//        return newfd;
//    currentTask->fd[newfd] = NULL;
//    currentTask->fd[newfd] = currentTask->fd[oldfd];
//    return newfd;
//}

int s_exev(char* binary_name, char *argv[]){
    //clear exisiting mm
    memset(getCurrentTask()->mm,0, sizeof(mm_struct));
    load_elf_binary_by_name(getCurrentTask(),binary_name,argv);
    return 1;
}

int syscall_handler(struct regs* reg) {
    int value = 0;
    int syscallNo = reg->rax;
    kprintf("syscall no %d\n",syscallNo);
    CURRENT_TASK = getCurrentTask();
    switch (syscallNo) {
        case SYSCALL_READ:
            value = sread(reg->rdi,reg->rsi,reg->rdx);
            break;
        case SYSCALL_WRITE:
            value = swrite(reg->rdi,reg->rsi,reg->rdx);
            break;
//        case SYSCALL_OPEN:
//            break;
//        case SYSCALL_FSTAT:
//            break;
//        case SYSCALL_LSEEK:
//            break;
//        case SYSCALL_CLOSE:
//            value = sclose(reg->rdi);
//            break;
//        case SYSCALL_BRK:
//            break;
//        case SYSCALL_PIPE:
//            break;
//        case SYSCALL_DUP2:
//            value = sdup2(reg->rdi,reg->rsi);
//            break;
//        case SYSCALL_GETPID:
//            value = sgetpid();
//            break;
//        case SYSCALL_FORK:
//            break;
//        case SYSCALL_EXECVE:
//            break;
        case SYSCALL_EXIT:
//            break;
//        case SYSCALL_WAIT4:
//            break;
//        case SYSCALL_GETCWD:
//            break;
//        case SYSCALL_GETDENTS:
//            break;
//        case SYSCALL_CHDIR:
//            break;
        default:
            kprintf("got a syscall\n");

    }
    return value;
}