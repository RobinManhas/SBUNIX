// Reference: http://www.osdever.net/bkerndev/Docs/pit.htm
#include <sys/kprintf.h>
#include <sys/idt.h>

unsigned int cycle = 0;
int awakeTime = 0;


void timer_handler()
{
    cycle++;
    if (cycle % 18 == 0) {
        awakeTime++;
        updateTimeOnScreen(awakeTime);
    }
}

void init_timer()
{
    irq_install_handler(0, &timer_handler);
}
