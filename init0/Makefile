.DEFAULT_GOAL := all
include ../make.conf

ARCHDIR=arch/$(ARCH)

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-omit-frame-pointer -DNDEBUG -std=c99
CPPFLAGS:=$(CPPFLAGS) -Iinclude -DARCH=\"$(ARCH)\" -DTARGET=X86 -DX86
LDFLAGS:=$(LDFLAGS) -Xlinker -Ttext=0x80000000
LIBS:=$(LIBS) -nostdlib -lgcc
NASMFLAGS:=-felf -I$(SYSROOT)/usr/include

include $(ARCHDIR)/make.conf

CFLAGS:=$(CFLAGS) $(INIT0_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(INIT0_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(INIT0_ARCH_LDFLAGS) -Xlinker -Map=init0.map
LIBS:=$(LIBS) $(INIT0_ARCH_LIBS)


.PHONY: all clean install install-kernel
.SUFFIXES: .o .c

INIT0_OBJS=\
	start.o\
	init0_main.o

.c.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	nasm $(NASMFLAGS) $< -o $@

all: init0.elf init0.bin

init0.elf: $(INIT0_OBJS) 
	$(CC) -o $@ $(LDFLAGS) $(INIT0_OBJS) $(LIBS)

init0.bin: init0.elf
	$(OBJCOPY) -O binary init0.elf init0.bin

install: install-init0 

install-init0: init0.elf init0.bin
	mkdir -p  $(SYSROOT)/boot
	cp init0.elf $(SYSROOT)/boot
	cp init0.bin $(SYSROOT)/boot

clean:
	rm -f init0.elf
	rm -f $(INIT0_OBJS)
	rm -f $(INIT0_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f init0.map
