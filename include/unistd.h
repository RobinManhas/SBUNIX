#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>




#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_FSTAT 5
#define SYSCALL_LSEEK 8
#define SYSCALL_CLOSE 3
#define SYSCALL_BRK 12
#define SYSCALL_PIPE 22
#define SYSCALL_DUP 32
#define SYSCALL_DUP2 33
#define SYSCALL_GETPID 39
#define SYSCALL_GETPPID 40
#define SYSCALL_FORK 57
#define SYSCALL_EXECVE 59
#define SYSCALL_EXIT 60
#define SYSCALL_WAIT4 61
#define SYSCALL_KILL 62
#define SYSCALL_GETCWD 79
#define SYSCALL_GETDENTS 78
#define SYSCALL_CHDIR 80
#define SYSCALL_SLEEP 81
#define SYSCALL_PS 90
#define SYSCALL_CLEARSCREEN 91


#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_CREAT 0x0100

int open(const char *pathname, int flags);
int close(int fd);
size_t sys_read(int fd, const void *buf, size_t count);
ssize_t sys_write(int fd, const void *buf, ssize_t count);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(int pid, int *status);

unsigned int sleep(unsigned int seconds);

int getpid();
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
//off_t lseek(int fd, off_t offset, int whence);
//int mkdir(const char *pathname, mode_t mode);
//int unlink(const char *pathname);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

#endif
