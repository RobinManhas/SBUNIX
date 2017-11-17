#ifndef _TARFS_H
#define _TARFS_H

#include <sys/defs.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

#define DIRECTORY 5
#define FILE 0
#define FILES_MAX 100

struct posix_header_ustar {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

typedef struct file_table file_table;
struct file_table{
    char* name;
    char type;
    uint64_t size;
    uint64_t current;
    uint64_t start;
    uint64_t end;
    file_table* child[FILES_MAX];
    int noOfChild;
    uint64_t inode;
};

file_table* tarfs[FILES_MAX];

typedef struct fd FD;
struct fd {
    //task_struct* current_process;
    uint64_t perm;
    uint64_t inode_no;
    file_table* filenode;
    uint64_t current_pointer;
};

file_table* getParentFolder(char* name, unsigned int len);


#endif
