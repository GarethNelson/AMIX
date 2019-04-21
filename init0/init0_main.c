#include <stdint.h>
#include <stddef.h>

#define X(syscall_num,syscall_name,params) __attribute__ ((naked)) uint32_t sys_ ## syscall_name params { __asm__ __volatile__ ("addl $8,%esp\n movl  $" #syscall_num ", %eax\n int $0x80\n subl $8, %esp"); }
	#include <amix/syscalls.def>
#undef X

void putchar(char c) {
	sys_debug_out((uint32_t)c);
}

void print_str(char* s) {
	while (*s) putchar(*s++);
}

void init0_main() {

	uint32_t my_tid = sys_get_tid();
	print_str("My TID: "); sys_debug_out_num(my_tid); print_str("\n");

	uint32_t fork_ret = sys_fork();
	if(fork_ret==0) {
		print_str("CHILD!\n");
	} else {
		print_str("PARENT!\n");
	}
}
