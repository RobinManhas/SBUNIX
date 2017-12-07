#include <unistd.h>
#include <sys/idt.h>
#include <sys/procmgr.h>
#include <sys/common.h>
#include<sys/kprintf.h>
#include <sys/mm.h>
#include <sys/pmm.h>
#include <sys/kstring.h>
#include <sys/kmalloc.h>
#include <sys/tarfs.h>

#define MSR_EFER 0xc0000080		/* extended feature register */
#define MSR_STAR 0xc0000081		/* legacy mode SYSCALL target */
#define MSR_LSTAR 0xc0000082 		/* long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084	/* EFLAGS mask for syscall */



extern void syscall_entry();
extern void forkChild();
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
    //kprintf("Inside syscall init\n");
    uint64_t efer = rdmsr(MSR_EFER)|0x1;
    wrmsr(MSR_EFER, efer);

   // wrmsr(MSR_STAR, (uint64_t)0x8 << 32 | (uint64_t)0x23 << 48);
    //uint64_t val = ((uint64_t)0x23 << 56 | (uint64_t)0x1b << 48 | (uint64_t)0x10 << 40 | (uint64_t)0x8 << 32);
    uint64_t val = ((uint64_t)0x1b << 48 |(uint64_t)0x8 << 32);
    wrmsr(MSR_STAR,val);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);
    wrmsr(MSR_SYSCALL_MASK, 1<<9); //disable interrupts in syscall mode need to check this code
}



uint64_t sread(uint64_t fdn, uint64_t addr,uint64_t len) {
    uint64_t read_length = -1;
    if(fdn<MAX_FD && CURRENT_TASK->fd[fdn] != NULL)
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
    if(fdn<MAX_FD && CURRENT_TASK->fd[fdn] != NULL)
    len_write = CURRENT_TASK->fd[fdn]->fileOps->write_file(fdn,(char *)addr,len);
    return len_write;
}

void skill(/* kills the current active process */){
    killTask(CURRENT_TASK);
}

uint64_t sbrk(uint64_t pointer){

    mm_struct* mm = getCurrentTask()->mm;
    if (pointer == 0)
        return mm->brk;
    else{
        vm_area_struct* heap = find_vma(mm, mm->brk);
        if(heap == NULL){
#ifdef ERROR_LOGS_ENABLE
            kprintf("ERROR: Heap not found in brk\n");
#endif
            return 0;
        }
        mm->brk =  pointer;
        heap->vm_end = mm->brk+1;
    }
    return mm->brk;
}

uint64_t sclose(uint64_t fdn){
    if(fdn<MAX_FD && CURRENT_TASK->fd[fdn] != NULL) {
        CURRENT_TASK->fd[fdn]->fileOps->close_file(fdn);
        CURRENT_TASK->fd[fdn] = NULL;
    }
    return 1;
}
//
uint64_t sgetpid(){
    return CURRENT_TASK->pid;
}


uint64_t sdup2(uint64_t oldfd , uint64_t newfd){
    if (newfd == oldfd)
        return newfd;
    FD* newp = CURRENT_TASK->fd[newfd];
    if(newp){
        newp->fileOps->close_file(newfd);
    }
//    CURRENT_TASK->fd[newfd] = NULL;
    CURRENT_TASK->fd[newfd] = CURRENT_TASK->fd[oldfd];
    CURRENT_TASK->fd[newfd]->ref_count++;
    return newfd;
}

uint64_t sdup(uint64_t oldfd){
    uint64_t newfd;
    for(newfd = 0; newfd < MAX_FD; newfd++)
        if(CURRENT_TASK->fd[newfd] == NULL)
            break;
    return sdup2(oldfd,newfd);
}
int s_exev(uint64_t binary_name, uint64_t argv,uint64_t envp){
    //clear exisiting mm
    //memset(getCurrentTask()->mm,0, sizeof(mm_struct));
    load_elf_binary_by_name(getCurrentTask(),(char *)binary_name,(char **)argv,(char **)envp);
    switch_to_user_mode(NULL,getCurrentTask());
    return 1;
}
pid_t sfork() {
#ifdef DEBUG_LOGS_ENABLE
    kprintf("at fork");
    printPageCountStats();
#endif
    task_struct* parent = getCurrentTask();
    task_struct* child = getFreeTask();
    createUserProcess(child);
    child->init = 1;
    child->user_rip = parent->user_rip;
    child->curr_dir = parent->curr_dir;
    parent->preemptiveTime = parent->preemptiveTime/2;
    child->preemptiveTime = parent->preemptiveTime;
    //child->user_rsp = parent->user_rsp;
    //child->rsp = parent->rsp;


    //copy the file descriptor list and increment reference count
    int i = 0;
    while( i < MAX_FD && parent->fd[i] != NULL) {
//        FD* fd = (FD*) kmalloc_size(sizeof(FD));
//        fd->perm = parent->fd[i]->perm;
//        fd->filenode =  parent->fd[i]->filenode;
//        fd->current_pointer = parent->fd[i]->current_pointer;
//        fd->ref_count = ++parent->fd[i]->ref_count;
//        fd->fileOps = parent->fd[i]->fileOps;
//        child->fd[i]=fd;
        parent->fd[i]->ref_count++;
        child->fd[i]=parent->fd[i];
        i++;
    }

    if(copy_mm(parent,child) == -1){
#ifdef ERROR_LOGS_ENABLE
        kprintf("error while copying task");
#endif
        return -1;
    }

#ifdef DEBUG_LOGS_ENABLE
    kprintf("at fork after copy mm");
    printPageCountStats();
#endif

    child->parent  = parent;
    child->ppid = parent->pid;
    //child->curr_dir = parent->curr_dir;

    if(parent->child_list == NULL)
        parent->child_list = child;
    else {
        child->nextChild = parent->child_list;
        parent->child_list = child;
    }
    parent->no_of_children++;


    //copy kernel stack;
    uint64_t rsp ;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(rsp));
    //aligning down
    rsp = (rsp>>12)<<12;
    kmemcpy(child->stack, (uint64_t *)rsp, PAGE_SIZE);
    //kprintf("kscheduling child\n");
    //16-128-8

    child->rsp = (uint64_t)&child->stack[510];
    child->rsp = ALIGN_UP(child->rsp,PAGE_SIZE)-152;
    *(uint64_t *)(child->rsp) = (uint64_t)forkChild;
    int child_pid = child->pid;

//    //schedule the next process to front; parent will only run after child
//    removeTaskFromRunList(child);
//    addTaskToReady(child,0);
    schedule();

    return child_pid;
}

int sclearscreen(){
    clearScreen();
    return 0;
}

pid_t sgetppid() {
    if(!getCurrentTask()->parent)
        return 1;
    else
        return getCurrentTask()->parent->pid;
}

int schdir(uint64_t path) {
    file_table* dir = find_file_using_relative_path((char*)path,getCurrentTask()->curr_dir);
   if(dir == NULL ||dir->type == FILE){
        return -1;
    }
    //kprintf("relative path: %s\n",dir->name);
    getCurrentTask()->curr_dir = dir;
    //kprintf("path changed to: %s\n", getCurrentTask()->curr_dir->name);
    return 1;
}
int scwd(uint64_t path){
    char* curr_dir = getCurrentTask()->curr_dir->name;
    kmemcpy((void *)path,(void*)curr_dir,kstrlen(curr_dir)+1);
    return 1;
}
int sopen(uint64_t path, uint64_t flags){
    return open_file((char*)path,(int)flags);

}

int ssleep(uint64_t sec) {
    getCurrentTask()->parent->sleepTime = sec * 1000;
    getCurrentTask()->parent->state = TASK_STATE_SLEEP;
    removeTaskFromRunList(getCurrentTask()->parent);
    addTaskToSleep(getCurrentTask()->parent);
//    schedule();
    return 1;
}

int  sps(uint64_t buf, uint64_t length){
    return get_ps((char *)buf, (uint8_t) length);
}



uint64_t swaitpid(uint64_t pid, uint64_t status_p, uint64_t options) {
    if(pid<0){
        return -1;
    }
    task_struct *cur_task = getCurrentTask();
    int *status = (int *) status_p;

    //if status pointer is not null; set the status
    if (cur_task->no_of_children == 0) {
        if (status) *status = -1;
        return -1;
    }


    while (1) {
        if (pid > 0) {
            int childPresent = 0;
            for (task_struct *task = cur_task->child_list; task != NULL; task = task->nextChild) {
                if (task->pid == pid) {
                    childPresent = 1;
                    if (task->state == TASK_STATE_KILLED || task->state == TASK_STATE_ZOMBIE) {
                        //child to wait on is killed
                        if (status) *status = 0;
                        return pid;
                    }
                }
            }
            if (!childPresent) {
                if (status) *status = -1;
                return -1;
            }
        }

        cur_task->state = TASK_STATE_WAIT;
        schedule();
        //wait done
        if (pid == 0)
            break;


    }
    if (status) *status = 0;
    return pid;

}

int spipe(uint64_t pipefd){
    uint32_t *pipes = (uint32_t *)pipefd;
    if(!pipes)
        return -1;
    uint32_t read_fd, write_fd;

    for(read_fd = 0; read_fd < MAX_FD; read_fd++)
        if(CURRENT_TASK->fd[read_fd] == NULL)
            break;
    for(write_fd = read_fd + 1; write_fd < MAX_FD; write_fd++)
        if(CURRENT_TASK->fd[write_fd] == NULL)
            break;
    if(read_fd == MAX_FD || write_fd == MAX_FD)
        return -1;
    pipes[0] = read_fd;
    pipes[1] = write_fd;
    CURRENT_TASK->fd[read_fd] = (FD*)kmalloc();
    CURRENT_TASK->fd[write_fd] = (FD*)kmalloc();
    return init_pipe(CURRENT_TASK->fd[read_fd], CURRENT_TASK->fd[write_fd]);
}


int syscall_handler(struct regs* reg) {
    int value = -1;
    int syscallNo = reg->rax;
    //kprintf("syscall no %d\n",syscallNo);
    CURRENT_TASK = getCurrentTask();
    switch (syscallNo) {
        case SYSCALL_READ:
            value = sread(reg->rdi,reg->rsi,reg->rdx);
            break;
        case SYSCALL_WRITE:
            value = swrite(reg->rdi,reg->rsi,reg->rdx);
            break;
        case SYSCALL_OPEN:
            value = sopen(reg->rdi, reg->rsi);
            break;
        case SYSCALL_CLOSE:
            value = sclose(reg->rdi);
            break;
          case SYSCALL_BRK:
              value = sbrk(reg->rdi);
                break;
        case SYSCALL_DUP2:
            value = sdup2(reg->rdi, reg->rsi);
            break;
        case SYSCALL_DUP:
            value = sdup(reg->rdi);
            break;
        case SYSCALL_GETPID:
            value = sgetpid();
            break;
        case SYSCALL_FORK:
            value = sfork();
            break;
        case SYSCALL_EXECVE:
            value = s_exev(reg->rdi,reg->rsi,reg->rdx);
            break;
        case SYSCALL_EXIT:
            killTask(getCurrentTask());
            break;
        case SYSCALL_WAIT4:
            value = swaitpid(reg->rdi,reg->rsi,reg->rdx);
            break;
        case SYSCALL_KILL:
            value = killPID(reg->rdi,reg->rsi);
            break;
        case SYSCALL_GETCWD:
            value = scwd(reg->rdi);
            break;
        case SYSCALL_CHDIR:
            value = schdir(reg->rdi);
            break;
        case SYSCALL_SLEEP:
            value = ssleep(reg->rdi);
            break;
        case SYSCALL_PS:
            value = sps(reg->rdi,reg->rsi);
            break;
        case SYSCALL_PIPE:
            value = spipe(reg->rdi);
            break;
        case SYSCALL_CLEARSCREEN:
            value = sclearscreen();
            break;
        default:
#ifdef DEBUG_LOGS_ENABLE
            kprintf("got a syscall : %d\n",syscallNo);
#endif
            break;
    }
    return value;
}