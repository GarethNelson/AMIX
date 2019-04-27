#include <stdint.h>
#include <stddef.h>

#include <AMIX/syscalls.h>

void putchar(char c) {
     sys_debug_out((uint32_t)c);
}

void print_str(char* s) {
     while (*s) putchar(*s++);
}

static char* readline(uint32_t fd, char* buf) {
     char c;
     int i = 0;
     for(;;) {
         while(sys_read(fd,&c,1) != 1); // keep polling till we get a byte
         if ((c == '\b' || c == '\x7f') && i > 0) {
            putchar('\b');
            i--;
         } else if (c >= ' ') {
            putchar(c);
            buf[i++] = c;
         } else if (c == '\n' || c == '\r') {
            buf[i] = '\0';
	    putchar('\n');
	    return buf;
         }
     }
}

void main() {
	print_str("Starting shell....\n");

	uint32_t stdin = sys_open("/dev/console");

	char buf[1024];

	char* cwd="/";

	for(;;) {
		print_str(cwd); print_str("$ "); readline(stdin,buf);
		uint32_t fork_ret = sys_fork();
		if(fork_ret==0) {
			sys_exec(buf);
		} else {
			sys_wait_tid(fork_ret);
		}
	}
}

