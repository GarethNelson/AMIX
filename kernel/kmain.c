#include "stdio.h"
#include "hal.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"
#include "thread.h"
#include "x86/multiboot.h"
#include "elf.h"
#include "vmspace.h"

#include <stdint.h>
#include <stdbool.h>

extern void jump_usermode();

void user_enter() {
	for(;;);
}

extern char* default_usercode;
extern char* default_usercode_end;

void load_elf_file(char* image, size_t image_len) {

     Elf32_Ehdr* elf_header = (Elf32_Ehdr*)image;
     Elf32_Off   ph_offset  = elf_header->e_phoff;
     uintptr_t elf_ph_header_ptr = elf_header;
     elf_ph_header_ptr += (uint32_t)ph_offset;
     Elf32_Phdr* elf_ph_header = elf_ph_header_ptr;

     kprintf("Loading valid ELF...\n");
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
    
     // TODO: data, bss, etc, and map instead of memcpy

     map(0x80020000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
     map(elf_ph_header->p_vaddr,alloc_pages(PAGE_REQ_NONE,(image_len/4096)+8),(image_len/4096)+2,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);


     switch_address_space(new_space);

     __builtin_memcpy(elf_ph_header->p_vaddr,image+elf_ph_header->p_offset,image_len); 

     void (*func_ptr)() = elf_header->e_entry;
     kprintf("Jumping to entry...\n");

__asm__ volatile("  \
     mov $0x23, %%ax; \
     mov %%ax, %%ds; \
     mov %%ax, %%es; \
     mov %%ax, %%fs; \
     mov %%ax, %%gs; \
                   \
     mov %%esp, %%eax; \
     pushl $0x23; \
     pushl %%eax; \
     pushf; \
     pushl $0x1B; \
     push %0; \
     iret; \
     ": : "b"(func_ptr));

}

typedef struct tar_header_t {
	char filename[100]; //NUL-terminated
	char mode[8];
	char uid[8];
	char gid[8];
	char fileSize[12];
	char lastModification[12];
	char checksum[8];
	char typeFlag; //Also called link indicator for none-UStar format
	char linkedFileName[100];

	//USTar-specific fields -- NUL-filled in non-USTAR version
	char ustarIndicator[6]; //"ustar" -- 6th character might be NUL but results show it doesn't have to
	char ustarVersion[2]; //00
	char ownerUserName[32];
	char ownerGroupName[32];
	char deviceMajorNumber[8];
	char deviceMinorNumber[8];
	char filenamePrefix[155];
	char padding[12]; 
    
} tar_header_t;

typedef enum tar_type_t {
	TAR_TYPE_NORMAL_FILE       = 48,
	TAR_TYPE_HARD_LINK         = 49,
	TAR_TYPE_SYMBOLIC_LINK     = 50,
	TAR_TYPE_CHAR_DEV          = 51,
	TAR_TYPE_BLOCK_DEV         = 52,
	TAR_TYPE_DIRECTORY         = 53,
	TAR_TYPE_FIFO              = 54,
	TAR_TYPE_CONTIGUOUS_FILE   = 55,
	TAR_TYPE_VENDOR_EXT_A      = 65,
	TAR_TYPE_VENDOR_EXT_B      = 66,
	TAR_TYPE_VENDOR_EXT_C      = 67,
	TAR_TYPE_VENDOR_EXT_D      = 68,
	TAR_TYPE_VENDOR_EXT_E      = 69,
	TAR_TYPE_VENDOR_EXT_F      = 70,
	TAR_TYPE_VENDOR_EXT_G      = 71,
	TAR_TYPE_VENDOR_EXT_H      = 72,
	TAR_TYPE_VENDOR_EXT_I      = 73,
	TAR_TYPE_VENDOR_EXT_J      = 74,
	TAR_TYPE_VENDOR_EXT_K      = 75,
	TAR_TYPE_VENDOR_EXT_L      = 76,
	TAR_TYPE_VENDOR_EXT_M      = 77,
	TAR_TYPE_VENDOR_EXT_N      = 78,
	TAR_TYPE_VENDOR_EXT_O      = 79,
	TAR_TYPE_VENDOR_EXT_P      = 80,
	TAR_TYPE_VENDOR_EXT_Q      = 81,
	TAR_TYPE_VENDOR_EXT_R      = 82,
	TAR_TYPE_VENDOR_EXT_S      = 83,
	TAR_TYPE_VENDOR_EXT_T      = 84,
	TAR_TYPE_VENDOR_EXT_U      = 85,
	TAR_TYPE_VENDOR_EXT_V      = 86,
	TAR_TYPE_VENDOR_EXT_W      = 87,
	TAR_TYPE_VENDOR_EXT_X      = 88,
	TAR_TYPE_VENDOR_EXT_Y      = 89,
	TAR_TYPE_VENDOR_EXT_Z      = 90,
	TAR_TYPE_GLOBAL_METADATA   = 103,
	TAR_TYPE_EXTENDED_METADATA = 120,
} tar_type_t;




unsigned int decodeTarOctal(const char *in, size_t len)
{
 
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;
 
    for (j = len-1; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);
 
    return size;
 
}

void dump_tar_file_entry(tar_header_t* header) {
	if(header->typeFlag=='\0') header->typeFlag=TAR_TYPE_NORMAL_FILE;
	switch(header->typeFlag) {
		case TAR_TYPE_NORMAL_FILE:
			kprintf("\t%s\t% 8d\t%-32s\n",header->mode,
						decodeTarOctal(header->fileSize,12),
						header->filename);
		break;
		case TAR_TYPE_HARD_LINK:
			kprintf("\t%s\t%08d\t%-32s\t\t -> \t%s\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename,
					header->linkedFileName);
		break;
		case TAR_TYPE_SYMBOLIC_LINK:
			kprintf("\t%s\t%08d\t%-32s\t\t -> \t%s\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename,
					header->linkedFileName);
		break;
		case TAR_TYPE_CHAR_DEV:
			kprintf("\t%s\t%08d\t%-32s\t\t (CHARDEV) %d %d\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename,
					decodeTarOctal(header->deviceMajorNumber,8),
					decodeTarOctal(header->deviceMinorNumber,8));
		break;
		case TAR_TYPE_BLOCK_DEV:
			kprintf("\t%s\t%08d\t%-32s\t\t (BLKDEV)  %d %d\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename,
					decodeTarOctal(header->deviceMajorNumber,8),
					decodeTarOctal(header->deviceMinorNumber,8));
		break;
		case TAR_TYPE_DIRECTORY:
			kprintf("\t%s\t%08d\t%-32s\t\t(DIR)\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename);
		break;
		case TAR_TYPE_FIFO:
			kprintf("\t%s\t%08d\t%-32s\t\t (FIFO)\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename);

		break;
		case TAR_TYPE_CONTIGUOUS_FILE:
			kprintf("\t%s\t%08d\t%-32s\n",header->mode,
					decodeTarOctal(header->fileSize,12),
					header->filename);
		break;
		case TAR_TYPE_VENDOR_EXT_A ... TAR_TYPE_VENDOR_EXT_Z:
			kprintf("\t\t VENDOR EXTENSION %c \t\t\n",header->typeFlag);
		break;
		case TAR_TYPE_GLOBAL_METADATA:
			kprintf("\t\t GLOBAL METADATA\t\t\n");
		break;
		case TAR_TYPE_EXTENDED_METADATA:
			kprintf("\t\t GLOBAL METADATA\t\t\n");
		break;
		default:
			kprintf("\t\t UNKNOWN ENTRY TYPE\t\t\n");
		break;
	}
}

void hexDump(char *desc, void *addr, int len) 
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        kprintf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                kprintf("  %s\n", buff);

            // Output the offset.
            kprintf("  0x%08x ", addr+i);
        }

        // Now the hex code for the specific character.
        kprintf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        kprintf("   ");
        i++;
    }

    // And print the final ASCII bit.
    kprintf("  %s\n", buff);
}


void load_tar_file(char* image, size_t image_len) {
	tar_header_t* header = (tar_header_t*)image;
	uintptr_t header_addr = (uintptr_t)image;
	kprintf("Entries in TAR file:\n");
	int i=0;
	for(i=0; i<image_len; i++) {
		header = (tar_header_t*)header_addr;
		size_t header_size = decodeTarOctal(header->fileSize,12);
		if(header->filename[0]=='\0') {
			kprintf("\tEOF\n");
			break;
		} else {
			dump_tar_file_entry(header);
		}
		header_addr += ((header_size / 512) +1) * 512;
		if(header_size % 512) header_addr += 512;
	}
	kprintf("%d entries in TAR file\n",i);
}

bool tar_check_file(char* img) {
	tar_header_t* tar_header = (tar_header_t*)img;
	if(tar_header->ustarIndicator[0] != 'u') return false;
	if(tar_header->ustarIndicator[1] != 's') return false;
	if(tar_header->ustarIndicator[2] != 't') return false;
	if(tar_header->ustarIndicator[3] != 'a') return false;
	if(tar_header->ustarIndicator[4] != 'r') return false;
	return true;
}


extern vmspace_t kernel_vmspace;
void init_task_enter(multiboot_module_entry_t* mod) {
	size_t		mod_len = mod->mod_end - mod->mod_start;
	map(mod->mod_start,mod->mod_start,(mod_len / 4096)+2,0);
	char* mod_img = (char*)mod->mod_start;

	kprintf("Loading multiboot module %s...\n",mod->string);

	if(elf_check_file(mod_img)) {
		kprintf("Got a valid ELF image, loading...\n");
		load_elf_file(mod_img,mod_len);
	} else if(tar_check_file(mod_img)) {
		kprintf("Got a ustar format TAR file at %p, loading...\n",mod_img);
		load_tar_file(mod_img,mod_len);
	} else {
		kprintf("Do not understand format of module %s, skipping\n",mod->string);
	}

	for(;;) thread_sleep(thread_current());
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

     for(;;) thread_sleep(thread_current());
}
