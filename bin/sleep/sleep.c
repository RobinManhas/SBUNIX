#include <stdio.h>
#include <syscalls.h>

int main(int argc, char *argv[], char *envp[]){
    int ret = -1;
    if(argc>2) {
        char *s = argv[2];
        int n = 0;
        ret = 0;
        while (*s) {
            if (*s >= '0' && *s <= '9') {
                n = n * 10 + (*s - '0');
                s++;
            } else {
                ret = -1;
                break;
            }
        }
        if(ret == 0) {
            //ret = syscall1(SYSCALL_SLEEP, n);
            puts("I am sleeping\n");
            ret = sleep(n);

        }
    }
    return ret;


}

