#include <unistd.h>
#include <sys/idt.h>
#include <sys/procmgr.h>
#include <sys/common.h>
#include <sys/util.h>



uint64_t sread(uint64_t fdn, uint64_t addr,uint64_t len) {
    uint64_t read_length = 0;
    if (fdn == 0) {
        read_length = read_stokes(addr, len);
        return read_length;
    }
    //read_length = read_file(fdn,addr,len);
    //need to check inode for fs extra 10 points??
    return read_length;
}

uint64_t swrite(uint64_t fdn, uint64_t addr,uint64_t len){
    if (fdn == 1 || fdn == 2) {
        len = writeString((char *)addr, len);
        return len;
    }
    //need to check fd>2 for fs extra 10 points??
    return -1;
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


int syscall_handler(struct regs* reg) {
    int value = 0;
    int syscallNo = reg->rax;

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
//        case SYSCALL_EXIT:
//            break;
//        case SYSCALL_WAIT4:
//            break;
//        case SYSCALL_GETCWD:
//            break;
//        case SYSCALL_GETDENTS:
//            break;
//        case SYSCALL_CHDIR:
//            break;
    }
    return value;
}