#include <stdint.h>
#include <stddef.h>

#include <AMIX/syscalls.h>

uint32_t my_tid;

void putchar(char c) {
     sys_debug_out((uint32_t)c);
}

void print_str(char* s) {
	sys_debug_out_str(s);
	//	while (*s) putchar(*s++);
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

void init0_main() {
	my_tid = sys_get_tid();
	print_str("My TID: "); sys_debug_out_num(my_tid); print_str("\n");

	char buf[1024];
	uint32_t fd;
	uint32_t fork_ret = sys_fork();
	sys_debug_out_num(fork_ret);
	if(fork_ret==0) {
		print_str("This is the child talking using normal sys_debug_out\n");
		for(;;) {
			sys_debug_out(sys_read_ringbuf());
		}
	} else {
		print_str("This is the parent talking using normal sys_debug_out\n");
	}

	print_str("Testing VFS stuff...\n");
	fd = sys_open("/dev/console");

	print_str("Opened /dev/console with fd: "); sys_debug_out_num(fd); print_str("\n");

	char* s="This is a string written to the FD\n";
	sys_write(fd,s,__builtin_strlen(s));


	print_str("Type something> ");
	char* line = readline(fd,buf);
	print_str("You typed: "); print_str(buf); putchar('\n');



       	char* test_str="This is a message sent from parent to child\n";
	sys_write_ringbuf(fork_ret,test_str,__builtin_strlen(test_str));

	print_str("Testing normal file reading:\n");
	fd = sys_open("/test.txt");
	print_str("Opened /test.txt with fd: "); sys_debug_out_num(fd); print_str("\n");

	uint32_t len=sys_read(fd,buf,35);
	print_str("Contents of /test.txt: "); print_str(buf);
	
	print_str("Testing exit\n");
	fork_ret = sys_fork();
	if(fork_ret==0) {
		print_str("In child process, about to call sys_exit()\n");
		sys_exit();
		print_str("You should not see this");
		for(;;);
	} else {
		print_str("Waiting on child...\n");
		sys_wait_tid(fork_ret);
		print_str("Back in the parent!\n");
	}

	print_str("Testing exec\n");
	fork_ret = sys_fork();
	if(fork_ret==0) {
		print_str("Hello, i'm the child, i'm going to sys_exec and you should see the new process run\n");
		sys_exec("/bin/sh");
		print_str("You should not see this!\n");
		for(;;);
	} else {
		print_str("Waiting on child...\n");
		sys_wait_tid(fork_ret);
		print_str("Back in the parent!\n");
	}

}

