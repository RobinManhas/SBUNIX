#include <sys/kprintf.h>
#include <stdarg.h>
#include <sys/defs.h>
#include <sys/util.h>

#define LINE_LENGTH 160U
#define MAX_LINES 24U //3680,3840
#define TIMER_LINE 25U
#define SPACE_OFFSET 2
uint64_t videoOutBufAdd = 0xb8000;
char intToCharOut[100]; // RM: string could be 64 bit address space
int intToCharLen = 0;
static int charsWritten = 0;
int keyboardOffset = 2;

void move_csr() {
    uint16_t temp = (uint16_t)(charsWritten/2);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(temp&0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((temp>>8)&0xFF));
}

void checkOverflow(char** outBuf)
{
    if(charsWritten >= (LINE_LENGTH*MAX_LINES))
    {
        // point the writer to last wriateable line
        int lastLine = LINE_LENGTH*(MAX_LINES-1);
        charsWritten = lastLine;
        *outBuf = ((char*)videoOutBufAdd) + lastLine;

        // shift all the lines above
        char* destPtr = (char*)videoOutBufAdd;
        char* srcPtr = ((char*)videoOutBufAdd) + 160;
        int charCount = 0;
        while(charCount < lastLine)
        {
            *destPtr = *srcPtr;
            destPtr+=2;
            srcPtr+=2;
            charCount+=2;
        }

        // clear the last line
        for(int i = lastLine;i<=(LINE_LENGTH*MAX_LINES);i++)
        {
            *destPtr = ' ';
            destPtr+=2;
        }

    }

}

unsigned int stringlen(char *s){
    if(!s){
        return -1;
    }
    unsigned int d = 0;
    for(d=0;s[d]!='\0';d++);
    return d;
}

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

void clearScreen(){

    char* destPtr = (char*)videoOutBufAdd;
    for(int i = 0; i < 160U * 25U; i+=2) {
        destPtr[i] = ' ';
    }

    charsWritten = 0;
    intToCharLen = 0;
    keyboardOffset = 2;
    move_csr();
}

void updateTimeOnScreen(int time)
{

    int timerIndex = LINE_LENGTH*TIMER_LINE - keyboardOffset - SPACE_OFFSET;
    char* screenWriter = ((char*)videoOutBufAdd); // initialize to base
    char *replaceBufPtr;
    intToCharLen = 0;
    intToChar(time,10);
    screenWriter += (timerIndex - (2*intToCharLen));
    intToCharOut[intToCharLen++] ='\0';

    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
    {
        *screenWriter = *replaceBufPtr;
        checkOverflow(&screenWriter);
        screenWriter +=2;
        replaceBufPtr +=1;
    }
}



void keyboardLocalEcho(char* input)
{
    if(!input || *input == '\0')
        return;
    int size = stringlen(input);
    if(size > 2) // only for ctrl + *, rest should be 1 character
        size = 2;

    int offset = 2*size;
    if(offset != keyboardOffset)
    {
        keyboardOffset = offset;
        // cls
        char* eraser = ((char*)videoOutBufAdd) + LINE_LENGTH*MAX_LINES;
        for(int i = (LINE_LENGTH*MAX_LINES);i<=(LINE_LENGTH*TIMER_LINE);i++)
        {
            *eraser = ' ';
            eraser+=2;
        }
    }

    int timerIndex = LINE_LENGTH*TIMER_LINE - keyboardOffset;
    char* screenWriter = ((char*)videoOutBufAdd) + timerIndex;
    *screenWriter = *input;
    if(size == 2){
        screenWriter+=2;
        input+=1;
        *screenWriter = *input;
    }

}

void kprintf(const char *fmt, ...)
{
    if(!fmt || *fmt == '\0'){
        return;
    }


    char *inputBufPtr, *outputBufPtr;
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

    for(inputBufPtr = (char*)fmt, outputBufPtr = (char*)videoOutBufAdd+charsWritten; *inputBufPtr;)
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
                        charsWritten += 2;
                        checkOverflow(&outputBufPtr);
                        outputBufPtr +=2;
                        replaceBufPtr +=1;
                    }
                    inputBufPtr+=2;
                    break;
                }
                case 'c':
                {
                    *outputBufPtr = va_arg(arglist, int);
                    charsWritten += 2;
                    checkOverflow(&outputBufPtr);
                    outputBufPtr +=2;
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
                        charsWritten += 2;
                        checkOverflow(&outputBufPtr);
                        outputBufPtr +=2;
                        replaceBufPtr +=1;
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
//                    *outputBufPtr = '0';
//                    outputBufPtr += 2;
//                    charsWritten += 2;
//                    checkOverflow(&outputBufPtr);
//                    *outputBufPtr = 'x';
//                    outputBufPtr += 2;
//                    charsWritten += 2;
                    checkOverflow(&outputBufPtr);

                    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        charsWritten += 2;
                        checkOverflow(&outputBufPtr);
                        outputBufPtr +=2;
                        replaceBufPtr +=1;
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
                    outputBufPtr += 2;
                    charsWritten += 2;
                    checkOverflow(&outputBufPtr);
                    *outputBufPtr = 'x';
                    outputBufPtr += 2;
                    charsWritten += 2;
                    checkOverflow(&outputBufPtr);

                    for(replaceBufPtr = (char*) intToCharOut;*replaceBufPtr;)
                    {
                        *outputBufPtr = *replaceBufPtr;
                        charsWritten += 2;
                        checkOverflow(&outputBufPtr);
                        outputBufPtr +=2;
                        replaceBufPtr +=1;
                    }
                    inputBufPtr+=2;
                    break;
                }

                default: // RM: Unhandled, print as is
                {
                    *outputBufPtr = *(inputBufPtr+1);
                    charsWritten += 2;
                    checkOverflow(&outputBufPtr);
                    outputBufPtr +=2;
                    inputBufPtr +=2;
                    break;
                }

            }
        }
        else if(*inputBufPtr == '\n')
        {
            int forward = charsWritten%LINE_LENGTH;
            outputBufPtr += (LINE_LENGTH - forward);
            charsWritten += (LINE_LENGTH - forward);
            checkOverflow(&outputBufPtr);
            inputBufPtr += 1;
        }
        else if(*inputBufPtr == '\r')
        {
            int back = charsWritten%LINE_LENGTH;
            outputBufPtr -= back;
            charsWritten -= back;
            checkOverflow(&outputBufPtr);
            inputBufPtr += 1;
        }
        else
        {
            *outputBufPtr = *inputBufPtr;
            charsWritten += 2;
            checkOverflow(&outputBufPtr);
            outputBufPtr +=2;
            inputBufPtr+=1;
            //charCount+=1;
            //if(charCount%159 == 0)
            //{
            //  charCount = 0;
            //  line+=1;
            //}
        }
    }
    va_end(arglist);
}

void kputch(char c) {
    char *outputBufPtr = (char *) videoOutBufAdd + charsWritten;

    int forward;
    switch (c) {
        case '\n':
            forward = charsWritten % LINE_LENGTH;
            outputBufPtr += (LINE_LENGTH - forward);
            charsWritten += (LINE_LENGTH - forward);
            break;
        case '\b':
            outputBufPtr -= 2;
            *outputBufPtr = ' ';
            charsWritten -= 2;
            break;
        default:
            *outputBufPtr = c;
            charsWritten += 2;
    }
    checkOverflow(&outputBufPtr);
    move_csr();

}
