// Reference: http://www.osdever.net/bkerndev/Docs/keyboard.htm

#include <sys/util.h>
#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/defs.h>
#include <string.h>
#include <sys/common.h>
#include <sys/terminal.h>


#define ALT 0
#define LCONTROL  29
#define RCONTROL 0
#define LSHIFT  42
#define RSHIFT  54
#define LSHIFT_KEYDOWN  42
#define RSHIFT  54
#define CAPSLOCK  0
#define NUMLOCK  0
#define SCROLLLOCK  0

int shiftFlag = 0;
int ctrlFlag = 0;


void irq_install_handler(int irq, void (*handler)());

unsigned char keyValue[128] =
        {
                0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', //0-10
                '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', //11-20
                'y', 'u', 'i', 'o', 'p', '[', ']', '\n',            //21-28
                0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',     //29-38
                ';', '\'', '`', 0,		/* Left shift */
                '\\', 'z', 'x', 'c', 'v', 'b', //43-48
                'n', 'm', ',', '.', '/',  0,				/* Right shift */'*', //49
                0,	/* Alt */                                                       //57
                ' ',	/* Space bar */
                0,	/* Caps lock */
                0,	/* 59 - F1 key ... > */
                0,   0,   0,   0,   0,   0,   0,   0,
                0,	/* < ... F10 */
                0,	/* 69 - Num lock*/
                0,	/* Scroll Lock */
                0,	/* Home key */
                0,	/* Up Arrow */
                0,	/* Page Up */
                '-',
                0,	/* Left Arrow */
                0,
                0,	/* Right Arrow */
                '+',
                0,	/* 79 - End key*/
                0,	/* Down Arrow */
                0,	/* Page Down */
                0,	/* Insert Key */
                0,	/* Delete Key */
                0,   0,   0,
                0,	/* F11 Key */
                0,	/* F12 Key */
                0,	/* All other keys are undefined */
        };

void keyboard_handler() {
   //kprintf("inside keyboard handler \n");
    unsigned char scancode;

    scancode = inb(0x60);
    //kprintf("key pressed %x\n",scancode);


    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80) // release event key up
    {
        switch(scancode & 0x7F){
            case LSHIFT:
            case RSHIFT: //kprintf("shift key up ");
                shiftFlag = 0;
                break;
            case LCONTROL: //kprintf("control key up ");
                ctrlFlag = 0;
                break;
        }
    }
    else  // key down
    {
        switch(scancode){
            case LSHIFT:
            case RSHIFT: //kprintf("shift key down");
                shiftFlag = 1;
                return;
            case LCONTROL: //kprintf("control key down");
                ctrlFlag = 1;
                return;
        }

        char keyRecv = keyValue[scancode];
        char keyPressed[3];
        if(shiftFlag) {
            switch(keyRecv){
                case '1':{
                    keyRecv = '!';
                    break;
                }
                case '2':{
                    keyRecv = '@';
                    break;
                }
                case '3':{
                    keyRecv = '#';
                    break;
                }
                case '4':{
                    keyRecv = '$';
                    break;
                }
                case '5':{
                    keyRecv = '%';
                    break;
                }
                case '6':{
                    keyRecv = '^';
                    break;
                }
                case '7':{
                    keyRecv = '&';
                    break;
                }
                case '8':{
                    keyRecv = '*';
                    break;
                }
                case '9':{
                    keyRecv = '(';
                    break;
                }
                case '0':{
                    keyRecv = ')';
                    break;
                }
                case '-':{
                    keyRecv = '_';
                    break;
                }
                case '=':{
                    keyRecv = '+';
                    break;
                }
                case '[':{
                    keyRecv = '{';
                    break;
                }case ']':{
                    keyRecv = '}';
                    break;
                }
                case ';':{
                    keyRecv = ':';
                    break;
                }
                case ',':{
                    keyRecv = '<';
                    break;
                }
                case '.':{
                    keyRecv = '>';
                    break;
                }
                case '/':{
                    keyRecv = '?';
                    break;
                }
                case '`':{
                    keyRecv = '~';
                    break;
                }
                case '\\':{
                    keyRecv = '|';
                    break;
                }
                case '\'':{
                    keyRecv = '"';
                    break;
                }
                default:{
                    keyRecv -= 32;
                    break;
                }

            }

        }

        if(ctrlFlag) {
            keyPressed[0] = '^';
            keyPressed[1] = keyRecv;

            if(keyRecv=='C'){
                //need to kill foregrnd process;
                // and schedule another process;
                kprintf("Received SIGKILL ^C, killing current process\n");
               // killTask(getCurrentTask());
            }
            /*overwrite for ctrl i, j, m
            if(keyRecv == '\t')
                keyPressed[1] = 'i';
            else if(keyRecv == '\n')
                keyPressed[1] = 'j';
            else if(keyRecv == '\r')
                keyPressed[1] = 'm';*/

        }
        else{
            //send to terminal
            add_buffer(keyRecv);
            keyPressed[0] = keyRecv;
            keyPressed[1] = '\0';
        }
        keyPressed[2] = '\0';

        keyboardLocalEcho(keyPressed);


    }

}
void init_keyboard() {
    //kprintf("inside keyboard init \n");
    irq_install_handler(1, &keyboard_handler);

}
