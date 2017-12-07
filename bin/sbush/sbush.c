#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>



#define MAX_PIPES_SUPPORTED 10

//Need to remove comment
static char* INTERPRETER ;
static char* PS1Value;
static int isConsoleInput =1;
static char* expandedPrompt;

char* expandPS1();
char* modifyPath(char* input, char* replace);
char** prepareCharArray(char* cmd);
void printCommandPrompt();
void executeFile( char* filename);
int setPathVariable(char* str);
void setPS1(char* str);
void forkProcessing(char * path[], char * env[], int isBackgroundProcess);
char* cmd_array[5];

int  processCommand(char* str, int isBackgroundProcess);

void printCommandPrompt(){

    char* tmp = expandPS1();
    putVal(tmp);

}
void setPS1(char* str){

    if(str==NULL || strlen(str) < 1){
        puts("wrong command");
        return;
    }
    if(str[0] == '"'){
        str = str+1;
    }
    if(str[strlen(str)-1]=='"'){
        str[strlen(str)-1] = '\0';
    }
    strcpy(PS1Value,str);
}

char* expandPS1(){
    int count = 0, index =0;
    for(index = 0 ; PS1Value[index] != '\0' ; ){
        if(PS1Value[index] == 92){
            char* replacementString = (char*)malloc(MAX_READ_BYTES);
            switch(PS1Value[index+1]){
                case 'h': strcpy(replacementString,"hostname");
                    break;
                case 'd': strcpy(replacementString,"date");
                    break;
                case 'w':
                    getcwd(replacementString, sizeof(replacementString));
                    break;

                default: replacementString = "";
                    break;

            }
            expandedPrompt[count]='\0';
            strcat(expandedPrompt,replacementString);
            count = count + strlen(replacementString);
            index = index+2;
        }
        else{
            expandedPrompt[count++]= PS1Value[index++];
        }
    }
    expandedPrompt[count] = '\0';
    return expandedPrompt;
}

int handlePipe(char* str) {
    if (NULL == str) {
        return -1;
    }
    int pipes[2]; // RM: pipe[0] for read, 1: write

    char *saveptr;

    char *firstBin = NULL, *secondBin = NULL;
    firstBin = strtok_r(str, "|", &saveptr);
    firstBin = trimString(firstBin);


    int terminal_write_backup = dup(1);

    secondBin = strtok_r(NULL, "|", &saveptr);
    secondBin = trimString(secondBin);

    char *buf = (char *) malloc(100);
    char *buf2 = (char *) malloc(100);
    int piperet = pipe(pipes);

    if (piperet != 0) {
        puts("Pipe creation failed, returning");
        return -1;
    }

    dup2(pipes[1],1);
    forkProcessing(prepareCharArray(firstBin), NULL, 0);
    dup2(terminal_write_backup, 1);
    memset(buf, 0, 100);
    sys_read(pipes[0], buf, 100);
    strcpy(buf2, secondBin);
    strcat(buf2, " ");
    strcat(buf2, buf);
    forkProcessing(prepareCharArray(buf2), NULL, 0);
    return 0;
}

char* modifyPath(char* input, char* replace){
    int count=0,loop=0;char* retVal; char* inputPtr = input;
    for(loop=0;input[loop]!='\0';loop++){
        if(strncmp(inputPtr,"$PATH",5)==0){
            count++;
            loop = loop + 4; //(len of $PATH(5) - 1)
            inputPtr += 4;
        }
        else{
            inputPtr += 1;
        }
    }
    int replaceLen = strlen(replace);
    retVal = (char*)malloc(loop+ count*(replaceLen - 5)+1);
    loop = 0;
    while(*input)
    {
        if(strncmp(input,"$PATH",5)==0){
            strcpy(&retVal[loop],replace);
            loop += replaceLen;
            input += 5;
        }
        else{
            retVal[loop++] = *input++;
        }
    }
    return retVal;
}

int setPathVariable(char* trunk)
{
    if(strlen(trunk) < 14) // length of 'export PATH=:/'
    {
        puts("No path specified, returning");
        //if(isConsoleInput) printCommandPrompt();
        return -1;
    }
    char* oldPath = getenv("PATH");

    strtok(trunk,"=");
    trunk = strtok(NULL,"=");

    if(strncmp(trunk,"$PATH",5) == 0)
    {
        if(strlen(trunk) <= 7){ // length of $PATH:/
            puts("No new path specified after $PATH, returning");
            //if(isConsoleInput) printCommandPrompt();
            return -1;
        }
        if(oldPath == NULL)
        {
            trunk+=5; // no $path, hence no replacement required
        }
        else
        {
            trunk = modifyPath(trunk,oldPath);
        }

    }


    int retVal = setenv("PATH",trunk,1);
    //printf("return value: %s\n", trunk);
    //printf("ret val: %d, new path: %s\n",retVal,getenv("PATH"));
    return retVal;
}

void forkProcessing(char * path[], char * env[], int isBackgroundProcess){
   if(path==NULL){
        return;
    }
    pid_t childPID = fork();

    if(childPID == -1){
        puts("Error: could not fork");
        exit(1);
    }
    if(childPID == 0){ //child body
        childPID = getpid();
        execve(path[0], path, environ);
        puts("Error: command not found");
        exit(0);
    }
    //parent body
    if(isBackgroundProcess != 1){

        waitpid(childPID,NULL);
    }
    return;

}

int processCommand(char* str, int isBackgroundProcess){
    // built-in: exit
    if(str[0]=='\0' ){
        return 0;
    }
    if((strncmp(str,"quit",4) == 0)||(strncmp(str,"exit",4) == 0)||
       (strncmp(str,"q",1) == 0) || (strncmp(str,"^C",1) == 0))
    {
        return -1;
    }
    else if(strchr(str,'|')!=NULL){
        //printf("found pipe \n");
        handlePipe(str);
        //if(isConsoleInput) printCommandPrompt();
        return 0;
    }
    // built-in: handling cd command
    if(strncmp(str,"cd",2) == 0)
    {
        //char curDir[MAX_READ_BYTES];
        str = str +2;
        str = trimString(str);
        int result = chdir(str);
        if(result <0){
            puts("No such directory");
        }
    }
    else if(strncmp(str,"export PATH=",11) == 0)
    {
        setPathVariable(str);
    }
    else if(strncmp(str,"./",2) == 0){
            char* filename = str+2;
            executeFile(filename);

    }
    else if(strncmp(str,"clear",5) == 0){
        clearScr();
    }
    else if(strncmp(str,"export PS1",10) == 0 || strncmp(str,"PS1",3)==0){
        if(strlen(str) < 5){
            puts("Invalid input for PS1");
            return 0;
        }
        char* ps1 = strtok(str,"=");
        char* ps1Value = strtok(NULL,"=");
        if(ps1 == NULL ||ps1Value ==NULL){
            puts("Invalid input for PS1");
            return 0;
        }
        setPS1(trimString(ps1Value));


    }
    else{ // check for binary
        forkProcessing(prepareCharArray(str),NULL,isBackgroundProcess);
    }

    return 0;
}


int main(int argc, char *argv[], char *envp[]) {

    expandedPrompt = (char*)malloc(MAX_READ_BYTES);

    INTERPRETER = (char*)malloc(MAX_READ_BYTES);
    strcpy(INTERPRETER,argv[0]) ;

    PS1Value = (char*)malloc(MAX_READ_BYTES);
    strcpy(PS1Value,"sbush:\\w> ");

    isConsoleInput = 1;
    char* str = (char*)malloc(MAX_READ_BYTES);
    printCommandPrompt();

//    int value =10;
// //To understand parent/ child forking, keep for future use
//	puts("before fork");
//	int id = fork();
//	if(id == 0){
//		puts("inside child fork");
//		exit(1);
//	}
//	else
//		puts("inside parent fork");
//
//    value = 30;
//
//    if(value == 30) // changing a local variable 'value', ideally must result in page fault as COW set
//        puts("value");
//
//	if(id == 0){
//		puts("child still active");
//	}
//	puts("after fork");
//
//    //free(str); // TODO RM: Make sure this is dealloc to avoid memleaks (check other leaks)

    int isBackgroundProcess = 0;
    int length =0;
    while(gets(str) != NULL) {
        isBackgroundProcess =0;
        str = trimString(str);
        if(str[0]!='\n'){
            str = strtok(str,"\n");
            str = trimString(str);
            length = strlen(str);
            if(str[length-1]=='&'){
                isBackgroundProcess =1;
                str[length-1]='\0';
                str = trimString(str);
            }
            if(processCommand(str,isBackgroundProcess)== -1){
                break;
            }
        }
        printCommandPrompt();

    }
    return 0;
}
//Check this function
void executeFile(char* filename){
    isConsoleInput = 0;
    int file;
    char* str;
    char* token;
    int startOfFile = 1;
    char fileData[MAX_READ_BYTES];
    getcwd(fileData,MAX_READ_BYTES);
    strcat(fileData,filename);
    file = open(fileData,O_RDONLY);
    if(file<0){
        puts("Cannot open file");
        return;
    }
    str = (char*)malloc(MAX_READ_BYTES);

    while(filegets(str,MAX_READ_BYTES,file)>0){
        //printf("FIle contents %s",str);
        if(strncmp(str,"\n",1) == 0){
            continue;
        }
        token = strtok(str,"\n");
        token = trimString(token);
        if(startOfFile ){
            startOfFile =0;
            if(strncmp(token,"#!/",3) == 0){
                token = token+3;
                if(strncmp(token, INTERPRETER,strlen(token)) != 0){
                    puts("Interpreter error");
                    return;
                }
            }
            else{
                puts("No interpreter defined");
                return;
            }
            continue;
        }
        else if(token == NULL || strncmp(token,"#",1) == 0 || (strncmp(token , " ",strlen(token)) ==0 )){
            continue;
        }
        processCommand(token,0);
    }
    isConsoleInput = 1;
    close(file);

}
char* findFileinPath(char* file){
    char* full_path = malloc(100);
    int fd;
    if(file[0] == '/'){
        strcpy(full_path,file+1);
        fd = open(full_path, O_RDONLY);
        if(fd >= 0) {
            //file exists
            close(fd);
            return full_path;
        }else
            return NULL;
    }
    char* paths = getenv("PATH");
    char* saveptr1;
    char *token=strtok_r(paths,":",&saveptr1);

    while(token!=NULL){
        if(token[0]=='/'){
            token = token+1;
        }
        strcpy(full_path,token);
        strcat(full_path,"/");
        strcat(full_path,file);
        fd = open(full_path, O_RDONLY);
        if(fd >= 0) {
            //file exists
            close(fd);
            //putn(fd);
            return full_path;
        }
        token = strtok_r(NULL,":",&saveptr1);
    }
    printf("No such file:%s in PATH\n",file);
    return NULL;

}

char** prepareCharArray(char* str){


    if(str == NULL) {
        return NULL;
    }
    char*cmd = malloc(100);
    strcpy(cmd,str);
    char *token=strtok(cmd," ");

    char * filePath;
    //token = trimString(token);

    filePath = findFileinPath(token);
    if(filePath == NULL){
        puts("command not found");
        return NULL;
    }
    cmd_array[0] = (char *)malloc(100);
    strcpy(cmd_array[0],filePath);


    cmd_array[1] = (char *)malloc(MAX_READ_BYTES);
    strcpy(cmd_array[1],token);
    str = str+strlen(token);
    int i =2;
    while(*str !='\0' && str != 0){
        str = trimString(str);
        strcpy(cmd,str);
        token = strtok(cmd," ");
        cmd_array[i] = (char *)malloc(MAX_READ_BYTES);
        strcpy(cmd_array[i],token);
        str = str+strlen(token);

        i++;
    }
    //cmd_array[i]=(char *)malloc(1);
    cmd_array[i] = NULL;

    return cmd_array;
}
