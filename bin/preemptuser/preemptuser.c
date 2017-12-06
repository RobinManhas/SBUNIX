#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
    uint64_t x = 1;
    while(x) {
        if(++x == 0x10000000) {
            putVal("User did lx additions!");
            putn(x);
            puts(" ");
            x = 1;
        }
    }
    return 0;
}