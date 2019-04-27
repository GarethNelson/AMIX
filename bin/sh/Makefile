.DEFAULT_GOAL := all
include ../../make.conf

ARCHDIR=arch/$(ARCH)

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-omit-frame-pointer -DNDEBUG -std=c99 
CPPFLAGS:=$(CPPFLAGS) -Iinclude -DARCH=\"$(ARCH)\" -DTARGET=X86 -DX86 
LDFLAGS:=$(LDFLAGS) 
LIBS:=$(LIBS) -nostdlib -lgcc
NASMFLAGS:=-felf -I$(SYSROOT)/usr/include

include $(ARCHDIR)/make.conf

CFLAGS:=$(CFLAGS) $(SH_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(SH_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(SH_ARCH_LDFLAGS) -Xlinker -Map=sh.map
LIBS:=$(LIBS) $(SH_ARCH_LIBS)


.PHONY: all clean install install-kernel
.SUFFIXES: .o .c

SH_OBJS=\
	start.o\
	main.o

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	nasm $(NASMFLAGS) $< -o $@

all: sh

sh: $(SH_OBJS) 
	$(CC) -o $@ $(LDFLAGS) $(SH_OBJS) $(LIBS)

install: install-sh 

install-sh: sh
	mkdir -p  $(INITRD)/bin
	cp sh $(INITRD)/bin/

clean:
	rm -f sh.elf
	rm -f $(SH_OBJS)
	rm -f $(SH_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f sh.map
