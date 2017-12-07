#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv, char **envp) {
    int pipefd[2], i;
    char buffer[4096];

    pipe(pipefd);
    puts(" nn");
    putn(pipefd[0]);
    puts("ggg");
    int oldterminal = dup(1);
    dup2(pipefd[1],1);

    for (i = 0; i < 2; i++)
        sys_write(pipefd[1], "hello pipe", 10);
    dup2(oldterminal,1);
    for (i = 0; i < 3; i++) {
        /* Continue reading from the pipe until it is empty */
        sys_read(pipefd[0], buffer, 1024);
        //buffer[err] = '\0';
        //putVal("harsh");

        puts(buffer);
        //printf("Read %d bytes: '%s'\n", err, buffer);
    }
    close(pipefd[1]);
    close(pipefd[0]);
    return 0;
}
