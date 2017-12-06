// Reference: http://www.osdever.net/bkerndev/Docs/pit.htm
#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/util.h>
#include <sys/procmgr.h>
#include <sys/common.h>

unsigned int cycle = 0;
int awakeTime = 0;


void timer_handler()
{
    cycle++;
    if (cycle % 1000 == 0) {//modified for sleep milisecs from 18 to 1000
        awakeTime++;
        updateTimeOnScreen(awakeTime);
    }
    reduceSleepTime();
    task_struct *current = getCurrentTask();
    if(current!= NULL && current->type == TASK_USER){
        current->preemptiveTime--;
        if(current->preemptiveTime<=0){
            schedule();
        }
    }

}

void init_timer() {

    uint16_t val = (uint16_t) (1193182 / 1000);
    outb(0x43, 0x36);
    outb(0x40, (uint8_t) val);
    outb(0x40, (uint8_t) (val >> 8));
    irq_install_handler(0, &timer_handler);
}
