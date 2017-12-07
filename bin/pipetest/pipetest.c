#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv, char **envp) {
    int pipefd[2], i;
    char buffer[4096];

    pipe(pipefd);
   // int oldterminal = dup(1);
   // dup2(pipefd[1],1);
    int pid = fork();
    if(pid==0){
        close(pipefd[0]);
        for (i = 0; i < 2; i++)
            sys_write(pipefd[1], "hello pipe", 10);
   }else {
        //dup2(oldterminal, 1);
        close(pipefd[1]);
        for (i = 0; i < 3; i++) {
            sys_read(pipefd[0], buffer, 1024);
            //buffer[err] = '\0';
            //putVal("harsh");

            printf("%s\n",buffer);
            memset(buffer,0,1024);
        }
    }
//    close(pipefd[1]);
//    close(pipefd[0]);
    return 0;
}
