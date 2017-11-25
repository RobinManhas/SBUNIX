#include <sys/common.h>
#include <sys/tarfs.h>
#include <sys/defs.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/pmm.h>
#include <sys/procmgr.h>
#include <sys/kstring.h>

#define BUFFER_SIZE 500
int full_flag =0;
char buffer[BUFFER_SIZE];
int buf_pointer=0;
int buffer_length=0;
task_struct* task_assigned_to_terminal = NULL;

uint64_t read_terminal(int fdNo, uint64_t buf,int size){
    if(task_assigned_to_terminal != NULL){
        kprintf("Another task is blocked on Terminal\n");
        return 0;
    }
    task_assigned_to_terminal = getCurrentTask();
    memset(buffer,0,BUFFER_SIZE);
    buffer_length = (size>BUFFER_SIZE)?BUFFER_SIZE:size;
    full_flag =0;
    buf_pointer=0;


    while(!full_flag) {
        task_assigned_to_terminal->state = TASK_STATE_BLOCKED;
        schedule();
    }

    memcpy((void*)buf,(void*)buffer,buf_pointer);
    task_assigned_to_terminal=NULL;
    return buf_pointer;

}
uint64_t write_terminal(char* s,uint64_t write_len){
    int return_count = 0;
    while( write_len >0){
        kputch(*s++);
        write_len--;
        return_count++;
    }
    return return_count;
}
int close_terminal(int fdNo){
    kprintf("Cannot close terminal\n");
    return -1;
}

uint64_t dummy_read_file(int fdNo, uint64_t buf,int size){
    kprintf("Cannot read on stdout\n");
    return -1;
}
uint64_t dummy_write_file(char* s,uint64_t write_len){
    kprintf("Cannot write on stdin\n");
    return -1;
}

struct fileOps terminalOps_IN = {
        .read_file= read_terminal,
        .write_file = dummy_write_file,
        .close_file = close_terminal
};

struct fileOps terminalOps_OUT = {
        .read_file= dummy_read_file,
        .write_file = write_terminal,
        .close_file = close_terminal
};

// need to call for every process creation and assign it to fd = 0,1 and 2 initially;
FD* create_terminal_IN(){
    FD* filedesc = (FD*)kmalloc();
    filedesc->fileOps=&terminalOps_IN;
    return filedesc;
}

FD* create_terminal_OUT(){
    FD* filedesc = (FD*)kmalloc();
    filedesc->fileOps=&terminalOps_OUT;
    return filedesc;
}


void add_buffer(char c){
    //print on screen
    kputch(c);
    if(task_assigned_to_terminal != NULL) {
        if (c == '\n' || buf_pointer > buffer_length) {
            full_flag = 1;
            removeTaskFromBlocked(task_assigned_to_terminal);
            task_assigned_to_terminal->state = TASK_STATE_RUNNING;
            addTaskToReady(task_assigned_to_terminal);
            schedule();

        } else if (c == '\b') {
            buf_pointer--;

        } else
            buffer[buf_pointer++] = c;
    }

}

