#ifndef _DEFS_H
#define _DEFS_H

#define NULL ((void*)0)
#define KERNBASE 0xffffffff80000000UL // from linkerscript 0xffffffff80000000
#define VIRBASE  0x55550000000UL
#define VMABASE 0x2000UL
#define KERNMASK 0x000000000FFFFFFFUL

#define MM_STACK_START 0x00007ffffffffff8ULL
#define MM_STACK_END 0x0000555555554000ULL

#define TABLE_ENTRIES_MAX 512
typedef unsigned long  uint64_t;
typedef          long   int64_t;
typedef unsigned int   uint32_t;
typedef          int    int32_t;
typedef unsigned short uint16_t;
typedef          short  int16_t;
typedef unsigned char   uint8_t;
typedef          char    int8_t;

typedef uint64_t size_t;
typedef int64_t ssize_t;

typedef uint64_t off_t;

typedef uint32_t pid_t;

#endif
