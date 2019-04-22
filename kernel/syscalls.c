#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "syscalls.h"
#include "thread.h"
#include "hal.h"
#include "vfs.h"
#include "fs.h"

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

uint32_t sys_write_ringbuf(uint32_t dest_tid, char *s, uint32_t size) {
	thread_t* dest = (thread_t*)dest_tid;
	char_ringbuf_write(&dest->ringbuf, s, size);
	return 0;
}

uint32_t sys_read_ringbuf() {
	char c;
	int retval = char_ringbuf_read(&thread_current()->ringbuf,&c,1);
	if(retval==0) return 0;
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

uintptr_t (*syscalls_table[SYSCALL_COUNT+1])(uintptr_t,uintptr_t,uintptr_t,uintptr_t) = {
#define X(num,name,params) [num] &sys_##name,
        #include "syscalls.def"
#undef X
};

