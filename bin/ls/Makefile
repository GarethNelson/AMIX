.DEFAULT_GOAL := all
include ../../make.conf

ARCHDIR=arch/$(ARCH)

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-omit-frame-pointer -DNDEBUG -std=c99 
CPPFLAGS:=$(CPPFLAGS) -Iinclude -DARCH=\"$(ARCH)\" -DTARGET=X86 -DX86 
LDFLAGS:=$(LDFLAGS) 
LIBS:=$(LIBS) -nostdlib -lgcc
NASMFLAGS:=-felf -I$(SYSROOT)/usr/include

include $(ARCHDIR)/make.conf

CFLAGS:=$(CFLAGS) $(LS_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(LS_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(LS_ARCH_LDFLAGS) -Xlinker -Map=ls.map
LIBS:=$(LIBS) $(LS_ARCH_LIBS)


.PHONY: all clean install install-kernel
.SUFFIXES: .o .c

LS_OBJS=\
	start.o\
	main.o

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	nasm $(NASMFLAGS) $< -o $@

all: ls

ls: $(LS_OBJS) 
	$(CC) -o $@ $(LDFLAGS) $(LS_OBJS) $(LIBS)

install: install-ls 

install-ls: ls
	mkdir -p  $(INITRD)/bin
	cp ls $(INITRD)/bin/

clean:
	rm -f ls.elf
	rm -f $(LS_OBJS)
	rm -f $(LS_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f ls.map
