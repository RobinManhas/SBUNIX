//
// Created by Shweta Sahu on 11/1/17.
//
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <sys/kstring.h>
#include <sys/kmalloc.h>
#include <sys/util.h>
#include <sys/procmgr.h>
#include <sys/pmm.h>
#include <sys/common.h>

file_table* new_file_table(char* name, char type,uint64_t size, uint64_t first ,uint64_t end, file_table *parent_node,uint64_t inode) {

    file_table* newFile = (file_table *)kmalloc_size(sizeof(file_table));
    //kprintf("size of filetable %d\n", sizeof(file_table));
    kstrcpy(newFile->name,name);
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
uint64_t read_file(int fdNo, uint64_t buf,int size);
uint64_t write_file(int fdNo,char* s,uint64_t write_len);
int close_file(int fdNo);
struct fileOps tarfsOps = {
    .read_file= read_file,
    .write_file = write_file,
    .close_file = close_file
};

file_table* get_parent_folder(char* name, unsigned int len){
    //kprintf("inside getParentFolder: %s\n",name);
    len--;
    while(name[len] != '/') {
        len--;
        if(len == 0)
            return tarfs[0];
    }
    char* parent_name = kmalloc(); // to be changed;need to provide size
    kstrncpy(parent_name,name,len+1);
    //kprintf("parent folder is: %s\n",parent_name);
    for(int i =0;i<FILES_MAX ; i++){
        if(NULL == tarfs[i] || kstrlen(tarfs[i]->name) == 0)
            break;
        if(kstrcmp(tarfs[i]->name, parent_name)==0) {
            //kprintf("parent found");
            return tarfs[i];
        }
    }
    //kprintf("not found");
    return tarfs[0];
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
    tarfs[index++]=new_file_table("/", DIRECTORY, 0, 0, 0, parent,0);


   while(*pointer < *end_pointer){
        //kprintf("inside init_tarfs while loop\n");
        start = (struct posix_header_ustar*)(&_binary_tarfs_start+ tmp);
        pointer = (uint64_t *)start;

        if(kstrlen(start->name) == 0) {
            break;
        }
       //kprintf("filename %s\n",start->name);
       //kprintf("file size %s\n",start->size);
        size = octalToDecimal(kstoi(start->size));
        if(size% header_size != 0){
            tmp += (size  + (header_size - size %header_size)+ header_size);
        }
        else
            tmp += (size + header_size);

       length = kstrlen(start->name);
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
    for(int i =1 ; i <FILES_MAX; i++){
        if(NULL == tarfs[i] || kstrlen(tarfs[i]->name) == 0)
            break;
#ifdef DEBUG_LOGS_ENABLE
        kprintf("%s parent:%s\n",tarfs[i]->name, tarfs[i]->child[1]->name);
#endif

    }


}



int open_file(char* file, int flag){ // returns filedescriptor id
    FD* filedesc;
    task_struct* currentTask = getCurrentTask();
    if(kstrlen(file)>1 && file[0]=='/'){
        file = file+1;
    }
    for(int i =0;i<FILES_MAX ; i++) {
        if (NULL == tarfs[i] || kstrlen(tarfs[i]->name) == 0)
            break;
        if (kstrcmp(tarfs[i]->name, file) == 0) {
           // kprintf("file found\n");
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
            filedesc->fileOps=&tarfsOps;
            filedesc->ref_count=1;
            if(filedesc->filenode->type == FILE)
                filedesc->current_pointer = 0;
            else
                filedesc->current_pointer = 2;
            currentTask->fd[count]=filedesc;
            return count;
        }
    }
    kprintf("No such file:%s\n",file);
    return -1;

}

uint64_t read_file(int fdNo, uint64_t buf,int size) {
    task_struct *currentTask = getCurrentTask();
    FD *filedesc = currentTask->fd[fdNo];
    if(filedesc != NULL && filedesc->perm != O_WRONLY) {
        uint64_t read_current = filedesc->current_pointer;
        if (filedesc->filenode->type == FILE) {

            uint64_t offset = filedesc->filenode->start + sizeof(struct posix_header_ustar) + read_current;
            uint64_t file_size = filedesc->filenode->size;

            if (file_size < read_current) {
                return -1;
            } else if (file_size == read_current) {
                return 0;
            } else if (file_size - read_current < size) {
                size = file_size - read_current;

            }

            kmemcpy((void *) buf, (void *) offset, size);
            filedesc->current_pointer += size;
            return size;
        } else if (read_current >= 2 && filedesc->filenode->noOfChild > read_current) {
            char *name = get_name(filedesc->filenode->child[read_current]);
            size = kstrlen(name);
            kmemcpy((void *) buf, (void *) name, size+1);

            filedesc->current_pointer++;
            return size;

        }
    }
    return  -1;
}




int close_file(int fdNo){
    task_struct* currentTask = getCurrentTask();
    if(currentTask->fd[fdNo] != NULL){

        if(--currentTask->fd[fdNo]->ref_count < 1){
            deallocatePage((uint64_t)currentTask->fd[fdNo]);
            currentTask->fd[fdNo] = NULL;
        }

        return 1;
    }
    return -1;
}

uint64_t write_file(int fdNo,char* s,uint64_t write_len){
    // currently not supported for tarfs
    return -1;
}

file_table* find_tar(char *file_name){
    file_table* file = NULL;
    for(int i =0;i<FILES_MAX ; i++) {
        if (NULL == tarfs[i] || kstrlen(tarfs[i]->name) == 0)
            break;
        //kprintf("file :%s\n", tarfs[i]->name);

        if (kstrcmp(tarfs[i]->name, file_name) == 0){
            //kprintf("file found:");
            file = tarfs[i];
            break;
        }
    }
    return file;

}

//returns the name mentioned at last in file
char* get_name(file_table* child){
    if(child == NULL || child->name == NULL)
        return NULL;
    int l = kstrlen(child->name);
    char* t;
    char tmp[l+1];
    kmemcpy(tmp, child->name,l+1);

    l = l-1;
    if(child->type == DIRECTORY){
        t = tmp+l-1;
        tmp[l]='\0';
    }

    else
        t = tmp+l;
    while( *t != '/') {

        if(t==tmp)
            break;
        t--;
    }
    if(*t == '/')
        t++;

    return t;

}

file_table* get_child(file_table *dir, char *child_name){
    //kprintf("inside getChild : %s\n",child_name);
    int i,l;
    char* tmp;
    char* t;
    for( i =2; i< FILES_MAX; i++){
        if(dir->child[i] == NULL || kstrcmp(dir->child[i]->name,"")==0)
            return NULL;
        tmp = dir->child[i]->name;
        l = kstrlen(tmp)-1;
        if(dir->child[i]->type == DIRECTORY)
             t = tmp+l-1;
        else
            t = tmp+l;
        while( *t != '/') {

            if(t==tmp)
                break;
            t--;
        }
        if(*t == '/')
            t++;
        //kprintf("child: %s\n",t);
        if(kstrcmp(child_name,t) == 0){
            return dir->child[i];
        }


    }
    return NULL;
}

file_table* find_file_using_relative_path(char* p_path,file_table* curr_dir){
    if(p_path != NULL && kstrcmp(p_path,"/")==0){
        return tarfs[0];
    }
    if(!curr_dir)
        curr_dir=tarfs[0];

    int l = kstrlen(p_path);
    char path[100];
    memset((void*)path,0,100);
    kmemcpy((void*)path,(void*)p_path,l+1);
    if(path[l-1]!= '/'){
        path[l++]='/';
    }
    path[l]='\0';

    //already absolute
    if(path[0] == '/') {
        return find_tar(path+1);
    }


    char tmp[20];

    int i =0;
    int j;

    while(i < l){
        memset(tmp,0,20);
        j =0;
        while(i <l && path[i]!='/' ){
            tmp[j++]=path[i++];

        }
        //kprintf("out of while tmp:%s \n",tmp);
        if(path[i] == '/'){
            if(kstrcmp(tmp,"")==0) {
                return NULL;
            }
            else if(kstrcmp(tmp,".")==0){
                //do nothing ??
                i++;//to increment '/'
            }
            else if(kstrcmp(tmp,"..")==0){
                curr_dir = curr_dir->child[1];//point to parent
                if(curr_dir == NULL){
                    return NULL;
                }
                //kprintf("curr_dir %s\n",curr_dir->name);
                i++;
            }
            else{
                tmp[j++] = path[i++];
                curr_dir = get_child(curr_dir, tmp);
                if(curr_dir == NULL){
                    return NULL;

                }
            }
        }

    }
    return curr_dir;

}
