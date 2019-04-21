#include <AMIX/syscalls.h>

extern putchar(uint32_t c);
void init0_main() {
	putchar('C');putchar('!');
}
