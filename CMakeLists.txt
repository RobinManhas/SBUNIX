cmake_minimum_required(VERSION 3.8)
project(OS_Project)

set(CMAKE_CXX_STANDARD 11)
include_directories(include/sys)
include_directories(include)
set(SOURCE_FILES
        crt/crt1.c
        include/sys/ahci.h
        include/sys/defs.h
        include/sys/elf64.h
        include/sys/gdt.h
        include/sys/idt.h
        include/sys/kprintf.h
        include/sys/pci.h
        include/sys/tarfs.h
        include/sys/util.h
        include/dirent.h
        include/signal.h
        include/stdarg.h
        include/stdio.h
        include/stdlib.h
        include/string.h
        include/syscalls.h
        include/unistd.h
        libc/stdio.c
        libc/stdlib.c
        libc/string.c
        libc/syscalls.c
        sys/gdt.c
        sys/idt.c
        sys/irq.c
        sys/keyboard.c
        sys/kprintf.c
        sys/main.c
        sys/pci.c
        sys/timer.c
        sys/util.c
        sys/pmm.c
        include/sys/pmm.h sys/kmalloc.c include/sys/kmalloc.h sys/vmm.c include/sys/vmm.h)

add_executable(OS_Project ${SOURCE_FILES})