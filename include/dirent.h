#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255
#include <sys/tarfs.h>
typedef struct dirent dirent;
struct dirent {
 char d_name[NAME_MAX+1];
};

typedef struct DIR DIR;
struct DIR{
    file_table* filenode;
    uint64_t curr;
    dirent curr_dirent;

};

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif
