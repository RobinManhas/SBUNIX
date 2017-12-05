//
// Created by Shweta Sahu on 12/5/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]){
    int size = 100;
    char *buf = malloc(100);


    int size_written = sys_ps(buf, size);
    if(size_written < 0)
        return -1;

    putVal("PID");putVal("    ");puts("CMD");
    char* saveptr1;
    char *token=strtok_r(buf,":",&saveptr1);
    while(token!=NULL){

        putVal(token);putVal("    ");
        token = strtok_r(NULL,":",&saveptr1);
        puts(token);
        token = strtok_r(NULL,":",&saveptr1);

    }
    return 1;
}

