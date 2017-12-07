#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscalls.h>
#include <unistd.h>


struct mem_block {
  size_t size;
  struct mem_block *next;
  int isfree;
};
struct mem_block *start_mem = NULL;


char *getenv(const char *name){
  short keyExists = 1;
  int keys = 0,  maxRunsThreshold = 3000;
  int inputEnvLen, keyLen;
  while(environ[keys] != '\0' && keys < maxRunsThreshold){
      if(environ[keys][0] != name[0])
      {
            keys+=1;
            continue; 
      }

      inputEnvLen = strlen(name);
      keyLen = 0;
      
      while(environ[keys][keyLen] != '=' && environ[keys][keyLen] != '\0'){
        keyLen++;
      }

      if(inputEnvLen != keyLen){
        keys+=1;
        continue;
      }  
      
      for(int i=0;i<keyLen;i++){
        if(environ[keys][i] != name[i])
        {
            keyExists = 0;
            break;
        }
      }

      if(keyExists){
          //puts(&environ[keys][keyLen+1]);
          return &environ[keys][keyLen+1];
      }

      keys+=1;
  }

    //puts("Not found");
    return NULL;


}

int setenv(const char *envname, const char *envval, int overwrite){
  //putVal("Setenv called for: ");puts(envval);putVal("\n");
  short keyExists = 0;
  int keys = 0, maxRunsThreshold = 3000; // key tells ID in environ table
  int inputEnvLen, keyLen; 
  while(environ[keys] != '\0' && keys < maxRunsThreshold)
  {
      if(environ[keys][0] != envname[0])
      {
            keys+=1;
            continue; 
      }

      inputEnvLen = strlen(envname);
      keyLen = 0;
      
      while(environ[keys][keyLen] != '=' && environ[keys][keyLen] != '\0'){
        keyLen++;
      }

      if(inputEnvLen != keyLen){
        keys+=1;
        continue;
      }  
      
      int matched = 1;
      for(int i=0;i<keyLen;i++){
        if(environ[keys][i] != envname[i])
        {
            matched = 0;
            break;
        }
      }

      if(matched){
        keyExists = 1;
        break;
      }

      keys+=1;
  }

  if(keyExists){ // append/overwrite to existing key based on input operation type
      if(overwrite){
          int space = keyLen + 1 + strlen(envval); // 1 for =
          
          char lastChar = envval[strlen(envval)];
          if(lastChar != '\0'){
            space += 1;
          }

          //char* new = (char*)malloc(space); // RM TODO: can use realloc in future here
          char* new = (char*)malloc(200); 
          strncpy(new,environ[keys],keyLen+1); // keylen + 1 so that '=' gets copied too
          strcat(new,envval);
          if(lastChar != '\0'){
            strcat(new,"\0");
          }

          environ[keys] = new;
          putVal("updated val: ");
          puts(environ[keys]);
      }
      else{
          int space = strlen(environ[keys]) + strlen(envval);
          
          char lastChar = envval[strlen(envval)];
          if(lastChar != '\0'){
            space += 1;
          }
          //char* new = (char*)malloc(space); // RM TODO: can use realloc in future here
          char* new = (char*)malloc(200);
          strcpy(new,environ[keys]);
          strcat(new,envval);
          if(lastChar != '\0'){
            strcat(new,"\0");
          }

          environ[keys] = new;
          putVal("updated val: ");
          puts(environ[keys]);
      }

          
  }
  else{ // new key value pair
      int space = strlen(envname) + 1 + strlen(envval);
      
      char lastChar = envval[strlen(envval)];
      if(lastChar != '\0'){
        space += 1;
      }
      //char* new = (char*)malloc(space); // RM TODO: can use realloc in future here
      char* new = (char*)malloc(200); 
      strcpy(new,envname);
      strcat(new,"=");
      strcat(new,envval);
      if(lastChar != '\0'){
        strcat(new,"\0");
      }

      environ[keys] = new;
      putVal("new key added: ");
      puts(environ[keys]);
      environ[keys+1] = '\0';
  }

  //puts(environ[keys]);
  return 0;

}



//void* malloc(size_t size){
//  return sys_brk(size);
//}
struct mem_block *addspace(struct mem_block* last_mem, size_t size) {
  struct mem_block *new_mem;
  new_mem = sys_brk(size + 20);
  if (last_mem) { 
    last_mem->next = new_mem;
  }
  new_mem->size = size;
  new_mem->next = NULL;
  new_mem->isfree = 0;
  return new_mem;
}

struct mem_block *search_mem(struct mem_block **last, size_t size) {
  struct mem_block *current = start_mem;
  while (current) {
    if(current->isfree){
      if(current->size >= size){
        break;
      }
    }
    *last = current;
    current = current->next;
  }
  return current;
}

void *malloc(size_t size) {
  struct mem_block *mem;
  if (size <1) {
    return NULL;
  }

  if (start_mem==NULL) {
    mem = addspace(NULL, size);
    if (!mem) {
      return NULL;
    }
    start_mem = mem;
  } else {
    struct mem_block *last = start_mem;
    mem = search_mem(&last, size);
    if (!mem) { 
      mem = addspace(last, size);
      if (!mem) {
        return NULL;
      }
    } else { 
      mem->isfree = 0;
    }
  }
  return(mem+1);
}

void free(void *ptr) {
  if (ptr==NULL) {
      puts("free: pointer was null");
    return;
  }
  struct mem_block* free_mem = (struct mem_block*)ptr - 1;
  free_mem->isfree = 1;
}

void exit(int status){
   syscall1(SYSCALL_EXIT,status);
}


