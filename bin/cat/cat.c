#include <stdio.h>

#define BUF_SIZE 1024

int catFile(char *file){
  int fd,ret;

  fd  = fileOpen(file, O_RDONLY);


  char fileData[BUF_SIZE];
  do {
    ret = sys_read(fd, fileData, BUF_SIZE);
    if(ret>0){
      sys_write(1,fileData,ret);
    }
  }while(ret>0);
  putchar('\n');
  close(fd);

  return 0;
}

int main(int argc, char *argv[], char *envp[]){
  if(argc >= 2 && argv[1]) {
      catFile(argv[1]);
  }
  return 0;
}