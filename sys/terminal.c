#include <sys/common.h>
#include <sys/tarfs.h>
#include <sys/defs.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>


uint64_t read_terminal(int fdNo, uint64_t buf,int size){
    return read_stokes(buf,size);
}
uint64_t write_terminal(char* s,uint64_t write_len){
    return writeString(s,write_len);
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


