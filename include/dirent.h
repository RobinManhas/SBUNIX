#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255
typedef struct dirent dirent;
struct dirent {
 char d_name[NAME_MAX+1];
};

typedef struct DIR DIR;
struct DIR{
    int filenode;
    uint64_t curr;
    struct dirent* curr_dirent;

};

DIR *opendir(char *name);
dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif
