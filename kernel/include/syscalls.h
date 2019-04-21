#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef KERNEL
	int sys_debug_out(char c);
	int sys_debug_out_num(uintptr_t n);
	uint32_t sys_get_tid();
	uint32_t sys_write_ringbuf(uint32_t dest_tid, char *s, uint32_t size);
	uint32_t sys_read_ringbuf();

#define SYSCALL_COUNT 5

	enum syscall_numbers {
	#define X(num,name,params) SYSCALL_##name=num,
		#include "syscalls.def"
	#undef X
	};

	extern uintptr_t (*syscalls_table[SYSCALL_COUNT+1])(uintptr_t,uintptr_t,uintptr_t,uintptr_t);
#else
	#define X(syscall_num,syscall_name,params) __attribute__ ((naked)) static inline uint32_t sys_ ## syscall_name params { __asm__ __volatile__ ("addl $8,%esp\n movl  $" #syscall_num ", %eax\n int $0x80\n subl $8, %esp"); }
		X(0,fork,()) // fork is a special case
		#include <amix/syscalls.def>
	#undef X

#endif
