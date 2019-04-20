#include "stdio.h"
#include "hal.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"
#include "thread.h"

extern void jump_usermode();

void user_enter() {
	for(;;);
}

extern char* default_usercode;
extern char* default_usercode_end;

int syscall_handler(struct regs *r, void *p) {
    debugger_except(r,"syscall triggered!");
}

void init_task_enter(void* p) {
     kprintf("init0: Running...\n");

     map(0x80002000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
     map(0x80000000,alloc_pages(PAGE_REQ_NONE,1),1,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);

     uint32_t usercode_len = (uint32_t)default_usercode_end - (uint32_t)default_usercode;
     __builtin_memcpy(0x80000000,&default_usercode,4096); 


     void (*func_ptr)() = 0x80000000;
     func_ptr();
     for(;;);
}

void kmain(int argc, char** argv) {
    kprintf("kmain() - Starting main OS...\n");

    thread_t* init_task = thread_spawn(&init_task_enter,NULL,1);
    for(;;);
}
