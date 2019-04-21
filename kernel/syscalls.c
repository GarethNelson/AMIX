#include <stddef.h>
#include <stdint.h>

#include "syscalls.h"
#include "thread.h"
#include "hal.h"

int sys_debug_out(char c) {
    kprintf("%c",c);
    return 0;
}

int sys_debug_out_num(uintptr_t n) {
    kprintf("0x%08x",n);
    return 0;
}

uint32_t sys_get_tid() {
    uint32_t tid = thread_current();
    return tid;
}

uintptr_t (*syscalls_table[SYSCALL_COUNT+1])(uintptr_t,uintptr_t,uintptr_t,uintptr_t) = {
#define X(num,name,params) [num] &sys_##name,
        #include "syscalls.def"
#undef X
};

