#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv, char **envp) {
    int pipefd[2], i;
    char buffer[4096];

    pipe(pipefd);
    puts(" nn");
    putn(pipefd[0]);
    puts("ggg");
    dup2(1, pipefd[1]);

    for (i = 0; i < 20; i++)
        sys_write(pipefd[1], "hello pipe", 10);
    for (i = 0; i < 100; i++) {
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
