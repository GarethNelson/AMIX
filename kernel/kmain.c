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

     Elf32_Ehdr* elf_header = (Elf32_Ehdr*)init0_img;
     Elf32_Off   ph_offset  = elf_header->e_phoff;
     uintptr_t elf_ph_header_ptr = elf_header;
     elf_ph_header_ptr += (uint32_t)ph_offset;
     Elf32_Phdr* elf_ph_header = elf_ph_header_ptr;

     kprintf("init0: Loading valid ELF...\n");
     kprintf("elf_header: %p, ph_offset: %p, ph_header: %p, entry: %p, ph_num: %p\n", elf_header, ph_offset,elf_ph_header,elf_header->e_entry,elf_header->e_phnum);

     kprintf("\t p_type=%p, p_offset=%p, p_vaddr=%p, p_paddr=%p, p_filesz=%p\n", elf_ph_header->p_type,
									elf_ph_header->p_offset,
									elf_ph_header->p_vaddr,
									elf_ph_header->p_paddr,
									elf_ph_header->p_filesz);

     address_space_t *new_space = kmalloc(sizeof(address_space_t));
     memset(new_space,0,sizeof(address_space_t));

     clone_address_space(new_space,1);
     kprintf("Setting up new vspace at %p\n",new_space->directory);

     switch_address_space(new_space);
     
     map(0x80020000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
     map(elf_ph_header->p_vaddr,alloc_pages(PAGE_REQ_NONE,(init0_len/4096)+2),(init0_len/4096)+2,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);


     switch_address_space(new_space);

     memcpy(elf_ph_header->p_vaddr,init0_img+elf_ph_header->p_offset,init0_len); 

     void (*func_ptr)() = ((Elf32_Ehdr*)init0_img)->e_entry; // 0x80000000;
     kprintf("Jumping to entry...\n");
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
