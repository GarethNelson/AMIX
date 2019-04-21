#pragma once
#include <stddef.h>
#include <stdint.h>
int sys_debug_out(char c);
int sys_debug_out_num(uintptr_t n);
uint32_t sys_get_tid();

#define SYSCALL_COUNT 3

enum syscall_numbers {
#define X(num,name,params) SYSCALL_##name=num,
	#include "syscalls.def"
#undef X
};

extern uintptr_t (*syscalls_table[SYSCALL_COUNT+1])(uintptr_t,uintptr_t,uintptr_t,uintptr_t);
