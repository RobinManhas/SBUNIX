#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
    uint64_t x = 1;
    while(x) {
        if(x == 0x10000000) {
            printf("Preempt User %d\n",getpid());
            x = 1;
        }
        x++;
    }
    return 0;
}