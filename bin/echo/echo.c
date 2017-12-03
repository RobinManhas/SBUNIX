#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]){
    for(int i=1 ; i < argc; i++) {
        puts(argv[i]);
        if(i!=argc-1)
            puts(" ");
    }
}