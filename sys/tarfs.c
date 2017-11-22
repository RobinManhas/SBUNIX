//
// Created by Shweta Sahu on 11/1/17.
//
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/kstring.h>
#include <sys/kmalloc.h>
#include <sys/util.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/procmgr.h>

file_table* new_file_table(char* name, char type,uint64_t size, uint64_t first ,uint64_t end, file_table *parent_node,uint64_t inode) {

    file_table* newFile = (file_table *)kmalloc_size(sizeof(file_table));
    //kprintf("size of filetable %d\n", sizeof(file_table));
    strcpy(newFile->name,name);
    newFile->type = type;
    newFile->size = size;
    newFile->current = first;
    newFile->start = first;
    newFile->end =end;
    newFile->child[0] = newFile;
    newFile->child[1]= parent_node;
    newFile->noOfChild = 2;
    newFile->inode = inode;
    if(parent_node != NULL)
        parent_node->child[parent_node->noOfChild++]=newFile;

    return newFile;
}

file_table* get_parent_folder(char* name, unsigned int len){
    //kprintf("inside getParentFolder: %s\n",name);
    len--;
    while(name[len] != '/') {
        len--;
        if(len == 0)
            return NULL;
    }
    char* parent_name = kmalloc(); // to be changed;need to provide size
    strncpy(parent_name,name,len+1);
    //kprintf("parent folder is: %s\n",parent_name);
    for(int i =0;i<FILES_MAX ; i++){
        if(NULL == tarfs[i] || strlen(tarfs[i]->name) == 0)
            break;
        if(strcmp(tarfs[i]->name, parent_name)==0) {
            //kprintf("parent found");
            return tarfs[i];
        }
    }
    //kprintf("not found");
    return NULL;
}

/**
 * Currently supporting only full path names
 * to support . and .. need to modify init_tarfs and opendir , change the tarfs from array to tree storing only name in name
 * not the full path
 */
void init_tarfs(){
    //kprintf("inside init_tarfs\n");
    struct posix_header_ustar* start = (struct posix_header_ustar*)&_binary_tarfs_start;
    struct posix_header_ustar* end = (struct posix_header_ustar*)&_binary_tarfs_end;

    uint64_t* pointer = (uint64_t*)start;
    uint64_t* end_pointer = (uint64_t*)end;
    uint64_t size =0;
    uint64_t header_size = sizeof(struct posix_header_ustar);
    uint64_t tmp = 0;
    file_table* newFile,*parent;
    parent = NULL;

    //file_table* current = tarfs;
    int length ;
    int index = 0;


   while(*pointer < *end_pointer){
        //kprintf("inside init_tarfs while loop\n");
        start = (struct posix_header_ustar*)(&_binary_tarfs_start+ tmp);
        pointer = (uint64_t *)start;

        if(strlen(start->name) == 0) {
            break;
        }
       //kprintf("filename %s\n",start->name);
       //kprintf("file size %s\n",start->size);
        size = octalToDecimal(stoi(start->size));
        if(size% header_size != 0){
            tmp += (size  + (header_size - size %header_size)+ header_size);
        }
        else
            tmp += (size + header_size);

       length = strlen(start->name);
        if(start->typeflag   == DIRECTORY) {
            //kprintf("processing for directory\n");
            parent = get_parent_folder(start->name,length-1);
            newFile = new_file_table(start->name, DIRECTORY, size, (uint64_t)pointer, tmp, parent,0);
        }
        else{
            //kprintf("processing for file\n");
           parent = get_parent_folder(start->name,length);
            //start will be pointer or pointer+header_size??? , test and check
            newFile = new_file_table(start->name,FILE,size,(uint64_t) pointer, tmp, parent,0);
        }
        tarfs[index++] = newFile;

    }
    //for debug purpose only
    for(int i =0 ; i <FILES_MAX; i++){
        if(NULL == tarfs[i] || strlen(tarfs[i]->name) == 0)
            break;
        kprintf("%s  %d  %d\n",tarfs[i]->name, tarfs[i]->type, tarfs[i]->noOfChild);

    }


}


/**
 * Currently supporting only full path names
 * to support . and .. need to modify init_tarfs and opendir , change the tarfs from array to tree storing only name in name
 * not the full path
 * @param name
 * @return
 */
DIR *opendir(const char *name){
    for(int i =0;i<FILES_MAX ; i++) {
        if (NULL == tarfs[i] || strlen(tarfs[i]->name) == 0)
            break;
        if ((strcmp(tarfs[i]->name, name) == 0) && tarfs[i]->type == DIRECTORY) {
            kprintf("directory found\n");
            DIR* dir = (DIR*)kmalloc();
            dir->filenode = tarfs[i];
            dir->curr = 2;
            return dir;
        }
    }
    kprintf("No such directory:%s\n",name);
    return NULL;

}

dirent *readdir(DIR *dirp){
    if(NULL == dirp || dirp->filenode->type != DIRECTORY)
        return NULL;
    if((dirp->curr >0) && (dirp->filenode->noOfChild > 2) && (dirp->curr < dirp->filenode->noOfChild) ){
        strcpy(dirp->curr_dirent.d_name , dirp->filenode->child[dirp->curr]->name);
        dirp->curr++;
        return &dirp->curr_dirent;
    }
    return NULL;

}
int closedir(DIR *dirp){
    if(NULL == dirp || dirp->filenode->type != DIRECTORY ||dirp->curr < 2)
        return -1;
    dirp->curr = 0;
    dirp->filenode = NULL;
//    dirp->curr_dirent = NULL;
    dirp = NULL;
    return 0;

}


int open_file(char* file, int flag){ // returns filedescriptor id
    FD* filedesc;
    for(int i =0;i<FILES_MAX ; i++) {
        if (NULL == tarfs[i] || strlen(tarfs[i]->name) == 0)
            break;
        if ((strcmp(tarfs[i]->name, file) == 0) && tarfs[i]->type == FILE) {
            kprintf("file found\n");
            filedesc =(FD*)kmalloc();
            //filedesc->current_process = currentTask;
            filedesc->perm = flag;
            filedesc->filenode = tarfs[i];
            int count =3;
            for(; count<MAX_FD; count ++){
                if(currentTask->fd[count] == NULL){
                    break;
                }
            }
            currentTask->fd[count]=filedesc;
            return count;
        }
    }
    kprintf("No such file:%s\n",file);
    return -1;

}

int read_file(int fdNo, uint64_t buf,int size){
    FD* filedesc = currentTask->fd[fdNo];
    if(filedesc != NULL && filedesc->perm != O_WRONLY){
        uint64_t read_current = filedesc->current_pointer;
        uint64_t end = filedesc->filenode->end;

        if (size > end - read_current) {
            size = end - read_current;
        }
        memcpy((void *) buf, (void *) read_current, size);
        filedesc->current_pointer += size;
        return  size;
    }
    kprintf("No fileDescriptor:%s\n",fdNo);
    return  -1;
}

int close_file(int fdNo){
    if(currentTask->fd[fdNo] != NULL){
        currentTask->fd[fdNo] = NULL;
        return 1;
    }
    return -1;
}

void* find_file(char* file_name){
    file_table* file = NULL;
    for(int i =0;i<FILES_MAX ; i++) {
        kprintf("file :%s\n", tarfs[i]->name);
        if (NULL == tarfs[i] || strlen(tarfs[i]->name) == 0)
            break;
        if ((strcmp(tarfs[i]->name, file_name) == 0) && tarfs[i]->type == FILE) {
            kprintf("file found:");
            file = tarfs[i];
            return (void*)file->start;
        }
    }
    kprintf("No such file:%s\n",file);
    return NULL;

}
