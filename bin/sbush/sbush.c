#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_READ_BYTES 100
#define MAX_PIPES_SUPPORTED 10

//Need to remove comment
static char* LIB_PATH ;
static char* INTERPRETER ;// could be replaced by some utility like getfullpath
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
char** prepareCharArray(char* cmd);
char* cmd_array[5];

int processCommand(char* str);

void printCommandPrompt(){

    char* tmp = expandPS1();
    putVal(tmp);
    /*for(int i=0;tmp[i]!='\0';i++) // Can't use puts as it adds newline by default
        putchar(tmp[i]);*/
}

void setPS1(char* str){

    if(str==NULL || strlen(str) < 1){
        puts("wrong command");
        return;
    }
    if(str[0] == '"'){
        //printf("starts with quote\n" );
        str = str+1;
        //printf("%s\n", str);
    }
    //printf(" end symbol%s", &str[strlen(str)-1] );
    if(str[strlen(str)-1]=='"'){
        //printf("ends with quote\n" );
        str[strlen(str)-1] = '\0';
        //printf("%s\n", str);
    }
    strcpy(PS1Value,str);
}

char* expandPS1(){
    //printf("inside expand");

    int count = 0, index =0;
    //printf("%s\t", PS1Value);
    for(index = 0 ; PS1Value[index] != '\0' ; ){
        //printf("inside for");
        if(PS1Value[index] == 92){
            //printf("inside if");
            char* replacementString = (char*)malloc(MAX_READ_BYTES);
            switch(PS1Value[index+1]){
                case 'h': strcpy(replacementString,"hostname");
                    break;
                case 'd': strcpy(replacementString,"date");
                    break;
                case 'w':
                    getdir(replacementString, sizeof(replacementString));
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
            //char* tmp = &PS1Value[index];
            //strcpy(expandedPrompt,tmp);
            expandedPrompt[count++]= PS1Value[index++];
        }
    }
    expandedPrompt[count] = '\0';
    return expandedPrompt;
}

int handlePipe(char* str){
    if(NULL == str){
        return -1;
    }
    pid_t identity;
    int fileDescriptor = 0; int pipes[2]; // RM: pipe[0] for read, 1: write

    char* saveptr;
    /*char* s = str; int count = 0;
    char* args[] = (char*)malloc(sizeof());
      for(int loop=0;s[loop]!='\0';loop++){
        if(s[loop] == '|'){
            count++;
        }
    }*/
    //printf("found pipes: %d, str: %s\n", count,str);
    char* firstBin = NULL, *secondBin = NULL;
    firstBin = strtok_r(str,"|",&saveptr);

    secondBin = strtok_r(NULL,"|",&saveptr);
    secondBin=trimString(secondBin);
    //printf("first: %s, second: %s\n",firstBin,secondBin);
    while(firstBin != NULL || secondBin != NULL){
        int piperet = pipe(pipes);
        if(piperet != 0){
            puts("Pipe creation failed, returning");
            return -1;
        }

        identity = fork();
        //printf("CP1: Fork returned: %d, first: %s, second: %s\n",identity, firstBin,secondBin);
        switch(identity){
            case -1:
            {
                //printf("identity invalid: first: %s, second: %s\n",firstBin,secondBin);
                exit(1);
                break;
            }
            case 0:
            {
                dup2(fileDescriptor,0);
                if(secondBin != NULL){
                    dup2(pipes[1],1); // RM: change the writing end to pipe
                }
                close(pipes[0]);
                //printf("sending for prepare: %s\n",firstBin);
                char** args = prepareCharArray(firstBin);
                char** env = {NULL};
                execve(args[0],args,env);
                exit(1);
                break;
            }
            default:{
                //printf("waiting: first: %s, second: %s\n",firstBin,secondBin);
                wait(NULL);
                close(pipes[1]);
                fileDescriptor = pipes[0];
                firstBin = secondBin;
                secondBin = strtok_r(NULL,"|",&saveptr);
                secondBin=trimString(secondBin);
                break;
            }
        }
    }
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

int setPathVariable(char* str)
{
    if(strlen(str) < 14) // length of 'export PATH=:/'
    {
        puts("No path specified, returning");
        //if(isConsoleInput) printCommandPrompt();
        return -1;
    }
    char* oldPath = getenv("PATH");
    char* trunk = strtok(str,"\n");

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
        //if(isConsoleInput) printCommandPrompt();
        exit(1);
    }
    if(childPID == 0){ //child body
        //int execlp(const char *file, const char *arg, ...); the last argument must be NULL
        //puts("inside child");
        //char *args[]={file,NULL};
        childPID = getpid();
        execve(path[0], path, env);
        puts("Error: command not found");
        exit(0);
    }
    //parent body
    if(isBackgroundProcess != 1){

        waitpid(childPID,NULL);
    }
    return;

}


int processCommand(char* str){
    // built-in: exit
    if((strncmp(str,"quit",4) == 0)||(strncmp(str,"exit",4) == 0)||
       (strncmp(str,"q",1) == 0) || (strncmp(str,"^C",1) == 0))
    {
        return -1;
    }
    else if(strchr(str,'|')!=NULL){
        //printf("found pipe \n");
        char* trunk = strtok(str,"\n");
        handlePipe(trunk);
        //if(isConsoleInput) printCommandPrompt();
        return 0;
    }
    // built-in: handling cd command
    if(strncmp(str,"cd",2) == 0)
    {
        //char curDir[MAX_READ_BYTES];
        char* trunk = strtok(str,"\n");
        strtok(trunk," ");
        trunk = strtok(NULL," ");
        int result = chdir(trunk);
        if(result <0){
            puts("Error: Invalid path");
        }
        // 0 success, -1 invalid path
        //printf(">> 1. pwd before : %s\n",getcwd(curDir, sizeof(curDir)));
        //printf(">> 2. chdir returned: %d\n",chdir(trunk));
        //printf(">> 3. pwd now : %s\n",getcwd(curDir, sizeof(curDir)));

    }
    else if(strncmp(str,"export PATH=",11) == 0)
    {
        setPathVariable(str);
        //printf("set path: %s\n", getenv("PATH"));
    }
    else if(strncmp(str,"./",2) == 0){
            char* filename = strtok(str,"\n")+2;
            executeFile(filename);

    }
        //check for PS1
    else if(strncmp(str,"export PS1",10) == 0 || strncmp(str,"PS1",3)==0){
        if(strlen(str) < 5){
            puts("invalid input for PS1");
            return 0;
        }
        char* input = strtok(str,"\n");
        char* ps1 = strtok(input,"=");
        char* ps1Value = strtok(NULL,"=");
        if(ps1 == NULL ||ps1Value ==NULL){
            puts("invalid input for PS1");
            return 0;
        }
        setPS1(trimString(ps1Value));


    }
    else{ // check for binary
        char* trunk = strtok(str,"\n");
        //readFile(trunk);
        forkProcessing(prepareCharArray(trunk),NULL,0);
    }

    return 0;
}


int main(int argc, char *argv[], char *envp[]) {
    //putchar(envp[0][0]);
   // long addrArg = (long)argv;
    //char* argvalues = (char*)addrArg;
//    LIB_PATH = (char*)malloc(MAX_READ_BYTES);
//    getdir(LIB_PATH,MAX_READ_BYTES);
//    strcat(LIB_PATH,"/");
    //strncpy(argvalues,argvalues,strlen(argvalues)-5);
   // strcat(LIB_PATH,argvalues);
    char* msg = "In sbushhhh";
    puts(msg);

    expandedPrompt = (char*)malloc(MAX_READ_BYTES);

    INTERPRETER = (char*)malloc(MAX_READ_BYTES);
    strcpy(INTERPRETER,"rootfs/bin/sbush") ;

    PS1Value = (char*)malloc(MAX_READ_BYTES);
    strcpy(PS1Value,"sbush:\\w> ");

    puts(PS1Value);
    char* str;
    isConsoleInput = 1;
    str = (char*)malloc(MAX_READ_BYTES);
    printCommandPrompt();
    while(gets(str) != NULL)
    {
        puts(str);
//        if(processCommand(str)== -1){
//            break;
//        }
//        printCommandPrompt();

    }
/* To understand parent/ child forking, keep for future use
	puts("before fork");
	int id = fork();
	if(id == 0){
		puts("inside child fork");
		exit(1);
	}
	else
		puts("inside parent fork");

	if(id == 0){
		puts("child still active");
	}
	puts("after fork"); */

    //free(str); // TODO RM: Make sure this is dealloc to avoid memleaks (check other leaks)
    return 0;
}
//Check this function
void executeFile(char* filename){
    isConsoleInput = 0;
    int file;
    char* str;
    char* token;
    int startOfFile = 1;
    file = fileOpen(filename,O_RDONLY);
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
            if(strncmp(token,"#!",2) == 0){
                token = token+2;
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
        processCommand(token);
    }
    isConsoleInput = 1;
    close(file);

}

char** prepareCharArray(char* cmd){
    if(cmd == NULL)
    {
        return NULL;
    }
    char* saveptr1;
    int i =1;
    //char a = '\0';
    char *token=strtok_r(cmd," ",&saveptr1);
    char * filePath = (char *)malloc(strlen(token)+strlen(LIB_PATH));
    strcpy(filePath,LIB_PATH);
    strcat(filePath,token);
    cmd_array[0] = (char *)malloc(40);
    while(token!=NULL){
        cmd_array[i] = (char *)malloc(MAX_READ_BYTES);
        strcpy(cmd_array[i],token);
        token = strtok_r(NULL," ",&saveptr1);
        i++;

    }

    //strcpy(cmd_array[i],&a);
    //strcpy(cmd_array[0],filePath);
    cmd_array[0]=filePath;
    //free(filePath);
    //execvp(*cmd_array,cmd_array);
    return cmd_array;
}
