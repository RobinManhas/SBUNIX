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
#define SYSCALL_DUP2 33
#define SYSCALL_GETPID 39
#define SYSCALL_FORK 57
#define SYSCALL_EXECVE 59
#define SYSCALL_EXIT 60
#define SYSCALL_WAIT4 61
#define SYSCALL_GETCWD 79
#define SYSCALL_GETDENTS 78
#define SYSCALL_CHDIR 80


#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_CREAT 0x0100

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
                        /* length is actually (d_reclen - 2 -
                           offsetof(struct linux_dirent, d_name) */

};


struct file_stat {
  unsigned long st_dev;  /*ID of device containing the file.*/
  unsigned long st_ino;  /*Serial number for the file.*/
  unsigned int st_mode;  /*Access mode and file type for the file*/
  unsigned long st_nlink;  /*Number of links to the file.*/
  unsigned int st_uid;  /*User ID of file owner.*/
  unsigned int st_gid;  /*Group ID of group owner.*/
  unsigned int st_rdev;  /*Device ID (if the file is a character or block special device).*/
  unsigned long  st_size;  /*File size in bytes */
  unsigned long  st_blksize;  /*A file system-specific preferred I/O block size for this object.*/
  unsigned long  st_blocks;  /*Number of blocks allocated for this file.*/
  unsigned long st_atime;  /*Time of last access.*/
  unsigned long st_mtime;  /*Time of last data modification.*/
  unsigned long st_ctime;  /*Time of last file status change.*/
};





int open(const char *pathname, int flags);
int close(int fd);
size_t sys_read(int fd, const void *buf, size_t count);
ssize_t sys_write(int fd, const void *buf, ssize_t count);
int unlink(const char *pathname);

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

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

#endif
