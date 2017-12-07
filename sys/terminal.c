#include <sys/common.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/pmm.h>
#include <sys/procmgr.h>
#include <sys/kstring.h>

#define BUFFER_SIZE 500

uint64_t read_terminal(int fdNo, uint64_t buf,int size);
int close_terminal_OUT(int fdNo);
uint64_t dummy_write_file(int fdNo,char* s,uint64_t write_len);
uint64_t dummy_read_file(int fdNo, uint64_t buf,int size);
int close_terminal_IN(int fdNo);
uint64_t write_terminal(int fdNo,char* s,uint64_t write_len);

int full_flag =0;
char buffer[BUFFER_SIZE];
int buf_pointer=0;
int buffer_length=0;
int totalchar = 0;
task_struct* task_assigned_to_terminal = NULL;

struct fileOps terminalOps_IN = {
        .read_file= read_terminal,
        .write_file = dummy_write_file,
        .close_file = close_terminal_IN
};

struct fileOps terminalOps_OUT = {
        .read_file= dummy_read_file,
        .write_file = write_terminal,
        .close_file = close_terminal_OUT
};

FD terminal_IN = {
        .fileOps=&terminalOps_IN,
        .perm=0,
        .filenode=0,
        .current_pointer=0,
        .ref_count=0,
        .pipenode=0
};

FD terminal_OUT = {
        .fileOps=&terminalOps_OUT,
        .perm=0,
        .filenode=0,
        .current_pointer=0,
        .ref_count=0,
        .pipenode=0
};

uint64_t read_terminal(int fdNo, uint64_t buf,int size){
    if(task_assigned_to_terminal != NULL){
#ifdef ERROR_LOGS_ENABLE
        kprintf("Another task is blocked on Terminal\n");
#endif
        return 0;
    }
    task_assigned_to_terminal = getCurrentTask();
    memset(buffer,0,BUFFER_SIZE);
    buffer_length = (size>BUFFER_SIZE)?BUFFER_SIZE:size;
    full_flag =0;
    buf_pointer=0;

    while(!full_flag) {
        task_assigned_to_terminal->state = TASK_STATE_BLOCKED;
        removeTaskFromReadyList(task_assigned_to_terminal);
        schedule();
    }

    kmemcpy((void*)buf,(void*)buffer,buf_pointer);
    task_assigned_to_terminal=NULL;
    return buf_pointer;

}
uint64_t write_terminal(int fdNo,char* s,uint64_t write_len){
    int return_count = 0;
    while( write_len >0){
        kputch(*s++);
        write_len--;
        return_count++;
    }
    return return_count;
}
int close_terminal_IN(int fdNo){
#ifdef ERROR_LOGS_ENABLE
    kprintf("Cannot close terminal\n");
#endif
    terminal_IN.ref_count--;
    return 0;
}

int close_terminal_OUT(int fdNo){
#ifdef ERROR_LOGS_ENABLE
    kprintf("Cannot close terminal\n");
#endif
    terminal_OUT.ref_count--;
    return 0;
}

uint64_t dummy_read_file(int fdNo, uint64_t buf,int size){
#ifdef ERROR_LOGS_ENABLE
    kprintf("Cannot read on stdout\n");
#endif
    return -1;
}
uint64_t dummy_write_file(int fdNo,char* s,uint64_t write_len){
#ifdef ERROR_LOGS_ENABLE
    kprintf("Cannot write on stdin\n");
#endif
    return -1;
}



// need to call for every process creation and assign it to fd = 0,1 and 2 initially;
FD* create_terminal_IN(){
//    FD* filedesc = (FD*)kmalloc();
//    filedesc->fileOps=&terminalOps_IN;
//    filedesc->ref_count= 1;
//    return filedesc;
    terminal_IN.ref_count++;
    return &terminal_IN;
}

FD* create_terminal_OUT(){
//    FD* filedesc = (FD*)kmalloc();
//    filedesc->fileOps=&terminalOps_OUT;
//    filedesc->ref_count= 1;
//    return filedesc;
    terminal_OUT.ref_count++;
    return &terminal_OUT;
}


void add_buffer(char c){
    //print on screen
    if(totalchar>0 && c=='\b'){
        kputch(c);
        totalchar--;
    }
    else if(c != '\b') {
        if (c == '\n') {
            totalchar = -1;
        }
        kputch(c);
        totalchar++;
    }

    if(task_assigned_to_terminal != NULL) {
        if (c == '\b' ) {
            if(buf_pointer>0){
                buf_pointer--;
            }
        }else if (c == '\n' || buf_pointer+1 == buffer_length) {
            full_flag = 1;

            buffer[buf_pointer++] = c;

            removeTaskFromBlocked(task_assigned_to_terminal);
            task_assigned_to_terminal->next = NULL;
            task_assigned_to_terminal->state = TASK_STATE_RUNNING;
            addTaskToReady(task_assigned_to_terminal,0);
            task_assigned_to_terminal = NULL;
            schedule();

        } else
            buffer[buf_pointer++] = c;
    }

}

