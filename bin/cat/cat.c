#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUF_SIZE 1024

int catFile(char *file) {
    int fd, ret;

    char fileData[BUF_SIZE];


    //puts(fileData);
    if(file!=NULL && file[0]!='/'){
        getdir(fileData,MAX_READ_BYTES);
        strcat(fileData,file);
    }else{
        strcpy(fileData,file);
    }

    fd = fileOpen(fileData, O_RDONLY);
    memset((void *)fileData,0,BUF_SIZE);

    do {
        ret = sys_read(fd, fileData, BUF_SIZE);
        if (ret > 0) {
            sys_write(1, fileData, ret);
        }
    } while (ret > 0);
    putchar('\n');
    close(fd);

    return 0;
}

int main(int argc, char *argv[], char *envp[]){
  if(argc > 2 && argv[1]) {
      catFile(argv[2]);
  }
  return 0;
}