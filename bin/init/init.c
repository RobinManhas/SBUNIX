#include <stdlib.h>
#include <stdio.h>

char * sbush_argv[] = {"bin/sbush", NULL};
char * sbush_envp[] = {"PATH=/bin:", "HOME=/root", "USER=root", NULL};
//
//char * test_argv[] = {"bin/preemptuser", NULL};
//char * test_envp[] = {"PATH=/bin:", "HOME=/root", "USER=root", NULL};

int main(int argc, char **argv, char **envp) {
//    int test = 0;
    while(1) {
        puts("starting SBUSH");

        pid_t childPID = fork();

        if (childPID == -1) {
            puts("Error: could not fork");
            //if(isConsoleInput) printCommandPrompt();
            exit(1);
        }
        if (childPID == 0) {
            execve(sbush_argv[0], sbush_argv, sbush_envp);
            exit(1);
        }
//        if(test==0) {
//            pid_t test = fork();
//            if (test == 0) {
//                execve(test_argv[0], test_argv, test_envp);
//                exit(1);
//            }
//        }
//        test = 1;
        waitpid(childPID, NULL);
        puts("SBUSH killed\n");
    }
    return 1;
}
