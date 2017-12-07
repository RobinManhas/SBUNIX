//
// Created by Shweta Sahu on 12/7/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv, char **envp) {
    puts("malloc for 100");
    char* tmp = (char*)malloc(100);
    puts("freeing 100");
    free(tmp);
    puts("free done");
    puts("allocating again 100");
    tmp = (char*)malloc(100);
    puts("malloc for 120");
    tmp = (char*)malloc(120);
    puts("freeing 120");
    free(tmp);

    puts("loop test start\n");
    int i=0;
    int* a;
    while(i<200){
         a = (int*)malloc(1000*sizeof(int));
        a[0]='1';
        free(a);
        i++;
    }
    puts("loop test done\n");
}

