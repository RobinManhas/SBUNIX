//
// Created by robin manhas on 12/7/17.
//

#include <stdio.h>
#include <stdarg.h>

char intToCharOut[100]; // RM: string could be 64 bit address space
int intToCharLen = 0;

void intToChar(int n, int base){
    if(n==0){
        return;
    }
    else if(n < 0)
    {
        intToCharOut[intToCharLen++] = '-';
        n *= -1;
    }
    int d = n%base;
    n = n/base;
    intToChar(n,base);
    intToCharOut[intToCharLen++] = (d > 9) ? (d-10) + 'a' : d + '0';
    return ;
}

void hextoChar(unsigned long long n, int base){
    if(n==0){
        return;
    }

    int d = n%base;
    n = n/base;
    hextoChar(n,base);
    intToCharOut[intToCharLen++] = (d > 9) ? (d-10) + 'a' : d + '0';
    return ;
}

int printf(const char *fmt, ...)
{
    int charsWritten = 0;
    char charOutBuf[1000];
    if(!fmt || *fmt == '\0'){
        return 0;
    }


    char *inputBufPtr,*outputBufPtr;
    char *retString;
    int retVal;
    va_list arglist; // RM: va_list not a method but a pre-processor directive, check make
    va_start(arglist,fmt);
    intToCharLen = 0;
    // Robin: enable this code to make kprintf similar to puts (de facto newline after kprintf stream)
    /*
    if(charsWritten%LINE_LENGTH != 0)
    {
        charsWritten += (LINE_LENGTH - (charsWritten%LINE_LENGTH));
        checkOverflow(&outputBufPtr);
    }*/

    for(inputBufPtr = (char*)fmt,outputBufPtr = (char*)charOutBuf; *inputBufPtr;)
    {
        // First filter
        if(*inputBufPtr == '%')
        {
            switch(*(inputBufPtr+1)){
                case 's':
                {
                    char *replaceBufPtr;
                    retString = va_arg(arglist, char *);
                    for(replaceBufPtr = (char*) retString;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        outputBufPtr+=1;
                        charsWritten+=1;
                        replaceBufPtr+=1;
                    }
                    inputBufPtr+=2;
                    break;
                }
                case 'c':
                {
                    *outputBufPtr = va_arg(arglist, int);
                    outputBufPtr+=1;
                    charsWritten+=1;
                    inputBufPtr+=2;
                    break;
                }
                case 'd':
                {
                    char *replaceBufPtr;
                    retVal = va_arg(arglist, int);
                    if(retVal == 0)
                    {
                        intToCharLen = 0;
                        intToCharOut[intToCharLen++] = retVal + '0';
                        intToCharOut[intToCharLen++] ='\0';
                    }
                    else
                    {
                        intToCharLen = 0;
                        intToChar(retVal,10);
                        intToCharOut[intToCharLen++] ='\0';
                    }

                    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        outputBufPtr+=1;
                        charsWritten+=1;
                        replaceBufPtr+=1;
                    }
                    inputBufPtr+=2;
                    break;
                }
                case 'p':
                {
                    char *replaceBufPtr;
                    unsigned long long returnVal;
                    returnVal = va_arg(arglist, unsigned long long);
                    if(returnVal == 0)
                    {
                        intToCharLen = 0;
                        intToCharOut[intToCharLen++] = returnVal + '0';
                        intToCharOut[intToCharLen++] ='\0';
                    }
                    else
                    {
                        intToCharLen = 0;
                        hextoChar(returnVal,16);
                        intToCharOut[intToCharLen++] ='\0';
                    }

                    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        outputBufPtr+=1;
                        charsWritten+=1;
                        replaceBufPtr+=1;
                    }
                    inputBufPtr+=2;
                    break;
                }
                case 'x':
                {
                    char *replaceBufPtr;
                    unsigned long long returnVal;
                    returnVal = va_arg(arglist, unsigned long long);
                    if(returnVal == 0)
                    {
                        intToCharLen = 0;
                        intToCharOut[intToCharLen++] = returnVal + '0';
                        intToCharOut[intToCharLen++] ='\0';
                    }
                    else
                    {
                        intToCharLen = 0;
                        hextoChar(returnVal,16);
                        intToCharOut[intToCharLen++] ='\0';
                    }
                    *outputBufPtr = '0';
                    outputBufPtr+=1;
                    charsWritten+=1;
                    *outputBufPtr = 'x';
                    outputBufPtr+=1;
                    charsWritten+=1;

                    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        outputBufPtr+=1;
                        charsWritten+=1;
                        replaceBufPtr+=1;
                    }
                    inputBufPtr+=2;
                    break;
                }

                default: // RM: Unhandled, print as is
                {
                    *outputBufPtr = *(inputBufPtr+1);
                    outputBufPtr+=1;
                    charsWritten+=1;
                    inputBufPtr+=2;
                    break;
                }

            }
        }
        else //handles \n and \r too
        {
            *outputBufPtr = *inputBufPtr;
            outputBufPtr+=1;
            charsWritten+=1;
            inputBufPtr+=1;

        }
    }

    sys_write(1,charOutBuf,charsWritten);
    charsWritten = 0;
    intToCharLen = 0;
    va_end(arglist);
    return 0;
}