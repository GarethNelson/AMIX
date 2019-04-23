#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "syscalls.h"
#include "thread.h"
#include "hal.h"
#include "vfs.h"
#include "fs.h"
#include "mmap.h"

int sys_debug_out(char c) {
    kprintf("%c",c);
    return 0;
}

int sys_debug_out_num(uintptr_t n) {
    kprintf("0x%08x",n);
    return 0;
}

uint32_t sys_debug_out_str(char* s) {
 	kprintf("%s",s);
}

uint32_t sys_get_tid() {
    uint32_t tid = thread_current();
    return tid;
}

uint32_t sys_write_ringbuf(uint32_t dest_tid, char *s, uint32_t size) {
	thread_t* dest = (thread_t*)dest_tid;
	char_ringbuf_write(&dest->ringbuf, s, size);
	thread_wake(dest);
	return 0;
}

uint32_t sys_read_ringbuf() {
	char c;
	int retval;
        while((retval = char_ringbuf_read(&thread_current()->ringbuf,&c,1))==0) thread_sleep();
	return (uint32_t)c;
}

bool check_access(int mode) {
	return true;
}

uint32_t sys_open(char* filename) {
	inode_t* inode = vfs_open(filename,&check_access);
        file_desc_t* fd = kmalloc(sizeof(file_desc_t));
        fd->inode = inode;
	fd->offs  = 0;
	vector_add(&thread_current()->fds,fd);
	vector_reserve(&thread_current()->fds,vector_length(&thread_current()->fds)+4);
	return vector_length(&thread_current()->fds)-1;
}

uint32_t sys_read(uint32_t fd, void* buf, uint32_t len) {
        file_desc_t* file_desc = vector_get(&thread_current()->fds,fd);
	inode_t* inode = file_desc->inode;
	uint64_t size = (uint64_t)size;
	uint32_t retval = (uint32_t)vfs_read(inode,file_desc->offs,buf,len);
	file_desc->offs += retval;
	return retval;
}


uint32_t sys_write(uint32_t fd, void* buf, uint32_t len) {
        file_desc_t* file_desc = vector_get(&thread_current()->fds,fd);
	inode_t* inode = file_desc->inode;
	uint64_t size = (uint64_t)size;
	uint32_t retval = (uint32_t)vfs_write(inode,file_desc->offs,buf,len);
	file_desc->offs += retval;
	return retval;
}

uint32_t sys_exit() {
	thread_wake(thread_current()->parent_task);
	thread_kill(thread_current());
	thread_yield();
	return 0; // we should never reach here in theory
}

uint32_t sys_wait_tid(uint32_t tid) {
	thread_t* other_task = (thread_t*)tid;
	while(other_task->state != THREAD_DEAD) thread_sleep();
	thread_destroy(other_task);
	return 0;
}

extern char* default_usercode;
extern char* default_usercode_end;

void exec_proc(char* filename) {
	// first let's scrap all user pages
	for(uintptr_t v=0x10000; v<MMAP_KERNEL_START; v=iterate_mappings(v)) {
		unsigned flags;
		get_mapping(v,&flags);
		if((flags & PAGE_USER)==PAGE_USER) { 
			uint64_t p = unmap(v,1);
		}
	}
	map(0x80020000,alloc_pages(PAGE_REQ_NONE,8),8,PAGE_USER|PAGE_WRITE);
	map(0x80000000,alloc_pages(PAGE_REQ_NONE,1),1,PAGE_USER|PAGE_WRITE|PAGE_EXECUTE);
	
	__builtin_memcpy(0x80000000,&default_usercode,4096);
	void (*func_ptr)() = 0x80000000;

	__asm__ volatile("  \ 
sti; \
 	mov $0x23, %%ax; \ 
     mov %%ax, %%ds; \ 
     mov %%ax, %%es; \ 
     mov %%ax, %%fs; \ 
     mov %%ax, %%gs; \ 
     mov %%esp, %%eax; \ 
     pushl $0x23; \ 
     pushl %%eax; \ 
     pushf; \ 
     pushl $0x1B; \ 
     push %0; \ 
     iret; \ 
     ": : "b"(func_ptr));

	kprintf("ERROR! Should never get here!\n");   
	return (uint32_t)func_ptr;
}

uint32_t sys_exec(char* filename) {
	thread_exec(&exec_proc, filename);
	return 0;
}

uintptr_t (*syscalls_table[SYSCALL_COUNT+1])(uintptr_t,uintptr_t,uintptr_t,uintptr_t) = {
#define X(num,name,params) [num] &sys_##name,
        #include "syscalls.def"
#undef X
};

