
#include <string.h>
#include <stdio.h>
#include <dirent.h>




int main(int argc, char *argv[], char *envp[]){
  char curDir[100];
  dirent *dp ;

  getdir(&curDir,100);

  DIR *dir;
  dir = opendir(curDir);
  dp =readdir(dir);
  if(!dp) return 0;
  while(dp != NULL){
    putVal(dp->d_name);
    putVal("    ");
    dp =readdir(dir);
  }
  putVal("\n");
  closedir(dir);
  return 0;
}






