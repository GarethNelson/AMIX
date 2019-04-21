#include "stdio.h"
#include "hal.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"
#include "thread.h"
#include "x86/multiboot.h"

extern void jump_usermode();

void user_enter() {
	for(;;);
}

extern char* default_usercode;
extern char* default_usercode_end;
char* init0_img;


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
     __builtin_memcpy(0x80000000,init0_img,4096); 

     void (*func_ptr)() = 0x80000000;
     func_ptr();
     for(;;);
}

extern multiboot_t mboot;


void setup_modules() {
	multiboot_t* mbi = &mboot;
      	if (mboot.flags & MBOOT_MODULES) {

	  multiboot_module_entry_t *mod;
           int i;
     
           kprintf ("mods_count = %d, mods_addr = 0x%x\n",
                   (int) mbi->mods_count, (int) mbi->mods_addr);
           for (i = 0, mod = (multiboot_module_entry_t *) mbi->mods_addr;
                i < mbi->mods_count;
                i++, mod++) {
             kprintf (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
                     (unsigned) mod->mod_start,
                     (unsigned) mod->mod_end,
                     (char *) mod->string);
		init0_img = mod->mod_start;
	   }

  }

}

void kmain(int argc, char** argv) {
     kprintf("kmain() - Starting main OS...\n");
     setup_modules();

    thread_t* init_task = thread_spawn(&init_task_enter,"A",1);

    for(;;) thread_yield();
}
