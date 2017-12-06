//
// Created by robin manhas on 12/6/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


int main(int argc, char *argv[], char *envp[]){
    int sig = SIGTERM, index = 2,ret=0;
    if(argc > 2 && argv[2]) {
        if(argv[2][0] == '-')
        {
            if((argv[2][1]) >= '0' && (argv[2][1]) <= '9')
            {
                sig = (int)stoi(argv[2] + 1); //fetch the signal
                index+=1;
            }
            else{
                puts("invalid signal passed for kill.");
            }
        }

        if(index == argc)
            return 1; //error

        for(;index < argc; index++) {
            int pid = (int)stoi(argv[index]);
            ret = kill(pid, sig);
            if(ret){
                puts("Error in kill pid");
            }
        }

    }
}