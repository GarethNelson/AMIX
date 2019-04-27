#include <stdint.h>
#include <stddef.h>

#include <AMIX/syscalls.h>


void putchar(char c) {
     sys_debug_out((uint32_t)c);
}

void print_str(const char* s) {
     while (*s) putchar(*s++);
}

void main() {
	print_str("ls /:\n");
	uint32_t dir_fd = sys_open("/");
	
	char buf[100];
	while(sys_read(dir_fd,buf,100)>0) {
		sys_debug_out_str(buf); print_str("\n");
	}
	print_str("done!\n");
	sys_exit();
}

