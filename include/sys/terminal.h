//
// Created by Shweta Sahu on 11/25/17.
//

#ifndef SBUNIX_TERMINAL_H
#define SBUNIX_TERMINAL_H

#include <sys/procmgr.h>


FD* create_terminal_IN();
FD* create_terminal_OUT();
void add_buffer(char c);
#endif //SBUNIX_TERMINAL_H
