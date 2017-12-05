#include <stdlib.h>
#include <stdio.h>

char * sbush_argv[] = {"bin/sbush", NULL};
char * sbush_envp[] = {"PATH=/bin:", "HOME=/root", "USER=root", NULL};

int main(int argc, char **argv, char **envp) {
    pid_t childPID = fork();

    if(childPID == -1){
        puts("Error: could not fork");
        //if(isConsoleInput) printCommandPrompt();
        exit(1);
    }
    if(childPID == 0) {
        execve(sbush_argv[0], sbush_argv, sbush_envp);
        exit(1);
    }
    waitpid(childPID,NULL);
    puts("sbush killed\n");
    return 1;
}
