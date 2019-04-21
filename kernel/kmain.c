#include "stdio.h"
#include "hal.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"
#include "thread.h"
#include "x86/multiboot.h"
#include "elf.h"

extern void jump_usermode();

void user_enter() {
	for(;;);
}

extern char* default_usercode;
extern char* default_usercode_end;

void init_task_enter(multiboot_module_entry_t* mod) {
 char*		init0_img = mod->mod_start;
 size_t		init0_len = mod->mod_end - mod->mod_start;
kprintf("init0: %s\n",mod->string);

	if(!elf_check_file(init0_img)) {
		kprintf("Bad ELF file!\n");
		for(;;);
	}


     address_space_t *new_space = kmalloc(sizeof(address_space_t));
     memset(new_space,0,sizeof(address_space_t));

     clone_address_space(new_space,1);
     kprintf("Setting up new vspace at %p\n",new_space->directory);

     switch_address_space(new_space);
     
     map(0x80020000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
     map(0x80000000,alloc_pages(PAGE_REQ_NONE,(init0_len/4096)+2),(init0_len/4096)+2,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);


     switch_address_space(new_space);

     memcpy(0x80000000,init0_img+0x1000,init0_len); 

     void (*func_ptr)() = ((Elf32_Ehdr*)init0_img)->e_entry; // 0x80000000;
     kprintf("init0: entry at %p\n", func_ptr);
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
			thread_spawn(&init_task_enter,mod,1);
	   }

  }

}

void kmain(int argc, char** argv) {
     kprintf("kmain() - Starting main OS...\n");
     setup_modules();

//    thread_t* init_task = thread_spawn(&init_task_enter,"A",1);

    for(;;) thread_yield();
}
