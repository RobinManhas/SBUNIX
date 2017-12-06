//
// Created by robin manhas on 12/6/17.
//
#include <syscalls.h>
#include <stdio.h>
#include <signal.h>

int kill(pid_t pid, int sig) {
    return (int) syscall2(SYSCALL_KILL, (uint64_t)pid, (uint64_t)sig);
}

