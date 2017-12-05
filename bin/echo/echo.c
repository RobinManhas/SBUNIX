#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]){
    for(int i=2 ; i < argc; i++) {
        putVal(argv[i]);
        if(i!=argc-1)
            putVal(" ");
    }
    puts(" ");
}