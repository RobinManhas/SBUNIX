#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]){
    puts("echo inside");
    for(int i=2 ; i < argc; i++) {
        puts(argv[i]);
        if(i!=argc-1)
            puts(" ");
    }
}