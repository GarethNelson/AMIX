.DEFAULT_GOAL := all
include ../make.conf

ARCHDIR=arch/$(ARCH)

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-omit-frame-pointer -DNDEBUG -std=c99
CPPFLAGS:=$(CPPFLAGS) -Iinclude -DARCH=\"$(ARCH)\" -DTARGET=X86 -DX86 -DKERNEL
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc
NASMFLAGS:=-felf

include $(ARCHDIR)/make.conf

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) -Xlinker -Map=kernel.map
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)


.PHONY: all clean install install-kernel
.SUFFIXES: .o .c

ADT_OBJS=\
	adt/bitmap.o\
	adt/buddy.o\
	adt/hashtable.o\
	adt/ringbuf.o\
	adt/vector.o

KERNEL_OBJS=\
	$(KERNEL_ARCH_OBJS)\
	$(ADT_OBJS)\
	block_cache.o\
	console.o\
	cow.o\
	debugger.o\
	debugger_cmds.o\
	dev.o\
	directory_cache.o\
	dummy.o\
	early_pmm.o\
	fs_vfat.o\
	gcov.o\
	hal.o\
	kmalloc.o\
	locking.o\
	main.o\
	partition.o\
	pmm.o\
	readline.o\
	scheduler.o\
	slab.o\
	snprintf.o\
	stdio.o\
	stdlib.o\
	string.o\
	thread.o\
	vfs.o\
	vmspace.o\
	syscalls.o\
	elf.o\
	kmain.o


.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	nasm $(NASMFLAGS) $< -o $@

all: kernel.bin

kernel.bin: $(KERNEL_OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LDFLAGS) $(KERNEL_OBJS) $(LIBS)


install: install-kernel install-headers

install-kernel: kernel.bin
	mkdir -p  $(SYSROOT)/boot
	cp kernel.bin $(SYSROOT)/boot

install-headers:
	mkdir -p $(SYSROOT)/usr/include/AMIX
	cp -Rv include/* $(SYSROOT)/usr/include/AMIX/

clean:
	rm -f kernel.bin
	rm -f $(KERNEL_OBJS)
	rm -f $(KERNEL_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f kernel.map
