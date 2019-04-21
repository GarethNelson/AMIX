#include <stdint.h>
#include <stddef.h>

#include <AMIX/syscalls.h>

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
		print_str("This is the child talking using normal sys_debug_out\n");
		for(;;) {
			sys_debug_out(sys_read_ringbuf());
		}
	} else {
		char* test_str="This is a message sent from parent to child\n";
		sys_write_ringbuf(fork_ret,test_str,__builtin_strlen(test_str));
		print_str("This is the parent talking using normal sys_debug_out\n");
	}
}
