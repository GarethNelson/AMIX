.PHONY: all run-qemu clean

all: kernel init0
	make -C kernel install
	make -C init0 install

clean: kernel
	make -C kernel clean
	make -C init0 clean
	rm -rf sysroot/

run-qemu: all
	qemu-system-i386 -kernel sysroot/boot/kernel.bin -serial mon:stdio -m 4G -initrd sysroot/boot/init0.elf
