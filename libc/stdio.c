#include <string.h>
#include <syscalls.h>
#include <stdio.h>


static const int ERROR = -1;
static int isExecuting = -1;
static int isPutValExecuting = 0;

ssize_t sys_write(int fd, const void *buf, ssize_t count) {
return syscall3(SYSCALL_WRITE, fd, (long)buf, count);
}

size_t sys_read(int fd, const void *buf, size_t count) {
return syscall3(SYSCALL_READ, fd, (long)buf, count);
}

int putchar(int c)
{
  // write character to stdout
  if(sys_write(1, &c , 1)==1){
    return c;
  }
  return ERROR;
}

void putn(long n){
    if(n==0)
      return;
    int d = n%10;
    n = n/10;
    putn(n);
    char tmp = d+'0';
    putchar(tmp);  
    return ;
}

int putVal(const char *s){
  if(isPutValExecuting){
    return -1;
  }
  isPutValExecuting = 1;
  if(s == '\0' || *s == '\0'){
    isPutValExecuting = 0;
    return 0;
  }
  for( ; *s; ++s){
    if (putchar(*s) != *s) {
      isPutValExecuting = 0;
      return EOF;
    }
  }
  isPutValExecuting = 0;
  return 0;
}



int getdir(void* buf, int size){
  long ret = syscall2((long)SYSCALL_GETCWD,(long)buf,(long)50);
  return (int)ret;
}

int fileOpen(void *filename, unsigned int flag){
long mode = 000777;
return syscall3(SYSCALL_OPEN,(long)filename, flag , mode);
}

int puts(const char *s)
{
  //isExecuting = 0;
  if(isExecuting==1)
    return -1;
  int i =1;

  isExecuting = 1;
  if(s == '\0' || *s == '\0'){
    isExecuting = 0;
    return 0;
  }
  for( ; *s; ++s){
    if (putchar(*s) != *s){
      isExecuting = 0;
      return EOF;
    }
    i++;
  }
  isExecuting = 0;
  putchar(i);
  return (putchar('\n') == '\n') ? 0 : EOF;
}


size_t sys_execve(char* filename, char** args, char ** envs){
  return syscall3(SYSCALL_EXECVE,(long)filename, (long)args, (long) envs);
}

int execve(char *filename, char* args[], char* envs[]){

  if(sys_execve(filename, args, envs) < 0)
    return ERROR;
  return 0;
}


void* sys_brk(size_t size){
  void* currentPointer = (void* )syscall1(SYSCALL_BRK, 0);
  syscall1(SYSCALL_BRK, (long)currentPointer+size);
  return currentPointer;
}

int chdir(const char *buf){
  long ret = syscall1((long)SYSCALL_CHDIR,(long)buf);
  return (int)ret;
}

int close(int fd){
  return syscall1(SYSCALL_CLOSE,fd);
}

int sys_lseek(int fd, int offset, int position){
  return syscall3(SYSCALL_LSEEK,fd,offset,position);
}

int sys_fstat(int fd ,void* file){
  return syscall2(SYSCALL_FSTAT,fd,(long)file);
}


int dup2(int fd, int newfd){
  return syscall2(SYSCALL_DUP2,fd,newfd);
}

//int getch(){
//  int c ;
//  if(sys_read(1, &c, 1) > 0){
//    return c;
//  }
//  return ERROR;
//}

//char *gets(char* s) {
//  char read;
//  //str =s;
//  for( int count=0; ; count++){
//    read = getch();
//    if ( read == -1 || read == '\n'){
//      s[count]='\0';
//      break;
//    }
//    s[count]= read;
//  }
//  return s;
//}

pid_t fork(){
  return syscall0(SYSCALL_FORK);
}

//int filegets(char *str , int size,int fd){
//  char s='\0';
//  int i = 0;
//  int ret = sys_read(fd, &s, 1);
//  while(s!='\n' && ret!=0){
//    str[i]=s;
//    i++;
//    if(i>=size){
//      break;
//    }
//    ret = sys_read(fd, &s, 1);
//  }
//  str[i]='\0';
//  return ret;
//}

int pipe(int pipefd[2]){
  return syscall1(SYSCALL_PIPE,(long)pipefd);
}

int getpid(){
  return syscall0(SYSCALL_GETPID);
}

pid_t wait(int *status){
  //long pid = getPID();
  return syscall4(SYSCALL_WAIT4,0,(long)status,0,0);
}

int waitpid(int pid, int *status){
  return syscall4(SYSCALL_WAIT4,(long)pid,(long)status,0,0);
}


