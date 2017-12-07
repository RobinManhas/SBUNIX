#include <sys/tarfs.h>
#include <sys/kmalloc.h>
#include <sys/procmgr.h>
#include <sys/pmm.h>



uint64_t dummy_read_pipe(int fdNo, uint64_t buf,int size){
    return -1;
}

uint64_t dummy_write_pipe(int fdNo,char* s,uint64_t write_len){
    return -1;
}

uint64_t write_pipe(int fdNo,char* s,uint64_t write_len){
    pipe_table* pipe = getCurrentTask()->fd[fdNo]->pipenode;
    if(pipe ==  NULL)
        return -1;
    if(pipe->read_closed)
        return -1;

    uint64_t count = 0;
    while(count < write_len) {
        while(pipe->full) {
            pipe->task_blocked->state = TASK_STATE_RUNNING;
            removeTaskFromBlocked(pipe->task_blocked);
            addTaskToReady(pipe->task_blocked,1);

            pipe->task_blocked = getCurrentTask();
            getCurrentTask()->state = TASK_STATE_BLOCKED;
            schedule();
        }
        if(pipe->read_closed)
            break;
        pipe->buf[pipe->end] = *s++;
        pipe->end = (pipe->end + 1) % MAX_BUFFER;
        if(pipe->end == pipe->start)
            pipe->full = 1;
        count++;
    }
    return count;
}
int write_close_pipe(int fdNo){
    pipe_table *pipe = getCurrentTask()->fd[fdNo]->pipenode;
    if (pipe == NULL)
        return -1;
    pipe->write_closed = 1;
//    if (pipe->read_closed) {
//        //deallocatePage((uint64_t )pipe);
//    } else {
        if(pipe->task_blocked){
            pipe->task_blocked->state = TASK_STATE_RUNNING;
            removeTaskFromBlocked(pipe->task_blocked);
            pipe->task_blocked->next = NULL;
            addTaskToReady(pipe->task_blocked, 0);
            pipe->task_blocked = NULL;
//        }

    }
    if(--getCurrentTask()->fd[fdNo]->ref_count<1)
        deallocatePage((uint64_t )getCurrentTask()->fd[fdNo]);
    return 0;
}


uint64_t read_pipe(int fdNo, uint64_t buf,int size){
    char* char_buf = (char*)buf;

    pipe_table* pipe = getCurrentTask()->fd[fdNo]->pipenode;
    if(pipe ==  NULL)
        return -1;
    while(pipe->start == pipe->end && !pipe->full) {
        if(pipe->write_closed) {//no more write
            return 0;
        } else {
            pipe->task_blocked = getCurrentTask();
            getCurrentTask()->state = TASK_STATE_BLOCKED;
            schedule();
        }
    }
    //pipe is full
    uint64_t isFull = pipe->full;
    uint64_t count = 0;
    while(count < size && pipe->start != pipe->end){
        char_buf[count] = pipe->buf[pipe->start];
        //char_buf++;
        pipe->start = (pipe->start + 1) % MAX_BUFFER;
        pipe->full = 0;
        count++;
    }

    if(isFull && pipe->task_blocked!=NULL) {
       //unblock task waiting on it
        pipe->task_blocked->state = TASK_STATE_RUNNING;
        removeTaskFromBlocked(pipe->task_blocked);
        addTaskToReady(pipe->task_blocked,0);
        pipe->task_blocked =NULL;
    }
    return count;

}

int read_close_pipe(int fdNo){
    pipe_table *pipe = getCurrentTask()->fd[fdNo]->pipenode;
    if (pipe == NULL)
        return -1;
//    pipe->read_closed = 1;
//    if (pipe->write_closed) {
//        //deallocatePage((uint64_t )pipe);
//    } else {
        if(pipe->task_blocked){
            pipe->task_blocked->state = TASK_STATE_RUNNING;
            removeTaskFromBlocked(pipe->task_blocked);
            pipe->task_blocked->next = NULL;
            addTaskToReady(pipe->task_blocked, 0);
            pipe->task_blocked = NULL;
//        }

    }
    if(--getCurrentTask()->fd[fdNo]->ref_count<1)
        deallocatePage((uint64_t )getCurrentTask()->fd[fdNo]);
    return 0;
}

struct fileOps read_end = {
        .read_file= read_pipe,
        .write_file = dummy_write_pipe,
        .close_file = read_close_pipe
};

struct fileOps write_end = {
        .read_file= dummy_read_pipe,
        .write_file = write_pipe,
        .close_file = write_close_pipe
};

int init_pipe(FD* readOut , FD* writeIn) {
    readOut->fileOps = &read_end;
    writeIn->fileOps = &write_end;
    writeIn->ref_count = 1;
    readOut->ref_count = 1;

    pipe_table* pt = (pipe_table*)kmalloc();
    readOut->pipenode = pt;
    writeIn->pipenode = pt;

    return 0;
}

