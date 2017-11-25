#ifndef __KPRINTF_H
#define __KPRINTF_H

void clearScreen();
void kprintf(const char *fmt, ...);
void updateTimeOnScreen(int time);
void keyboardLocalEcho(char* input);
void kputch(char c);
#endif
