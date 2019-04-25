#include "hal.h"
#include "elf.h"
#include "dev_initrd.h"
#include "x86/multiboot.h"

extern multiboot_t mboot;

static void initrd_describe(block_device_t* bdev, char* buf, unsigned bufsz) {
	dev_initrd_t* initrd = (dev_initrd_t*)bdev->data;
	ksnprintf(buf, bufsz, "initrd block device from %s, %dkb big", initrd->filename, initrd->image_len/1024);
}

static uint64_t initrd_length(block_device_t* bdev) {
	dev_initrd_t* dev = (dev_initrd_t*)bdev->data;
	return dev->image_len;
}

static int initrd_read(block_device_t* bdev, uint64_t offset, void* buf, uint64_t len) {
	dev_initrd_t* dev = (dev_initrd_t*)bdev->data;

	uintptr_t image_buf = (uintptr_t)dev->image;
	__builtin_memcpy(buf,image_buf+offset,len);
	return len;
}

int dev_initrd_init() {
	multiboot_t* mbi = &mboot;
	multiboot_module_entry_t *mod;
	int i;
    
        for (i = 0, mod = (multiboot_module_entry_t *) mbi->mods_addr;
                i < mbi->mods_count;
                i++, mod++) {
			size_t mod_len = mod->mod_end - mod->mod_start;
			map(mod->mod_start,mod->mod_start,(mod_len / 4096)+1,0);
			char* mod_img = mod->mod_start;
			if(!elf_check_file(mod_img)) {
				block_device_t* bdev = kmalloc(sizeof(block_device_t));
				dev_initrd_t* initrd = kmalloc(sizeof(dev_initrd_t));
				ksnprintf(initrd->filename,100, "%s", mod->string);
				initrd->image     = mod_img;
				initrd->image_len = mod_len;
				bdev->data     = initrd;
				bdev->read     = &initrd_read;
				bdev->write    = NULL;
				bdev->flush    = NULL;
				bdev->length   = &initrd_length;
				bdev->describe = &initrd_describe;
				bdev->id       = makedev(DEV_MAJ_INITRD, i);
				register_block_device(bdev->id,bdev);
			} else {
				/* skip it, it's not an initrd image, it's a task to load */
			}
	   }
	return 0;

}

static prereq_t prereqs[] = { {"vfs",NULL},
                              {NULL,NULL} };

static module_t x run_on_startup = {
  .name = "dev-initrd",
  .required = prereqs,
  .load_after = NULL,
  .init = &dev_initrd_init,
  .fini = NULL
};
