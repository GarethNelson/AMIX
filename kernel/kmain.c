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



void init_task_enter(char* str) {
     kprintf("init0: %s\n",str);

     address_space_t *new_space = kmalloc(sizeof(address_space_t));
     memset(new_space,0,sizeof(address_space_t));

     clone_address_space(new_space,0);
     kprintf("Setting up new vspace at %p\n",new_space->directory);

     switch_address_space(new_space);
     
     map(0x80002000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
     map(0x80000000,alloc_pages(PAGE_REQ_NONE,1),1,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);


     switch_address_space(new_space);

     uint32_t usercode_len = (uint32_t)default_usercode_end - (uint32_t)default_usercode;
     __builtin_memcpy(0x80000000,&default_usercode,4096); 


     void (*func_ptr)() = 0x80000000;
     func_ptr();
     for(;;);
}

void kmain(int argc, char** argv) {
    kprintf("kmain() - Starting main OS...\n");
    thread_t* init_task1 = thread_spawn(&init_task_enter,"A",1);
    thread_t* init_task2 = thread_spawn(&init_task_enter,"B",1);
    thread_t* init_task3 = thread_spawn(&init_task_enter,"C",1);
    thread_t* init_task4 = thread_spawn(&init_task_enter,"D",1);
    for(int i=0; i<64; i++) thread_spawn(&init_task_enter,"T",1);
    for(;;) thread_yield();
}
