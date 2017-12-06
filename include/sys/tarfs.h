#ifndef _TARFS_H
#define _TARFS_H

#include <sys/defs.h>


extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

#define DIRECTORY '5'
#define FILE '0'
#define FILES_MAX 100
#define MAX_BUFFER 2000


#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_CREAT 0x0100

struct posix_header_ustar {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag;
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
    char name[100];
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



file_table* get_parent_folder(char* name, unsigned int len);
file_table* find_tar(char *file_name);
void init_tarfs();
file_table* find_file_using_relative_path(char* p_path,file_table* curr_dir);
int open_file(char* file, int flag);
char* get_name(file_table* child);



#endif
