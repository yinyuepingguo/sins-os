#			SINS
#	This is Makefile for whole kernel.
#	You can type 'make help' to get some usage.

export TARGET	= boot/sins
export CC	= gcc
export AS	= gcc
export LD	= ld
export DASM	= objdump
#	There is a mass that Printer irq will happened when use -O0 rather than Ox
export CFLAGS	= -I $(shell pwd)/include -nostdinc -fno-builtin -Wall -O2
export ASMFLAGS = -I $(shell pwd)/include -nostdinc -fno-builtin -Wall -D__ASSEMBLY__
export DASMFLAGS = -D
export DASMFILE	= dasm.tmp
export LDFLAGS	= -nostdlib -nostdinc

BACKUP_DIR	= /var/sins.backup
MOUNT_DIR	= /var/sins.disk

objs	= arch/arch.o boot/boot.o init/init.o kernel/kernel.o	\
	 drivers/drivers.o lib/lib.o fs/fs.o

$(TARGET): $(objs)
	gcc -P  $(ASMFLAGS) boot/link.lds.S -E -o boot/link.lds
	ld $(LDFLAGS) -T $(shell pwd)/boot/link.lds -o $@ $^

arch/arch.o:
	cd arch; make

boot/boot.o:
	cd boot; make

init/init.o:
	cd init; make

kernel/kernel.o:
	cd kernel; make

fs/fs.o:
	cd fs; make

drivers/drivers.o:
	cd drivers; make

lib/lib.o:
	cd lib; make


.PHONY: help clean mount umount install backup dasm

help:
	@echo "  make install	----install sins to disk.img using by bochs"
	@echo "  make mount	----mount disk.img to /dev/loop0(need root)"
	@echo "  make umount	----umount disk.img from /dev/loop0(need root)"
	@echo "  make help	----print this menu"
	@echo "  make clean	----clean all generated files!"
	@echo "  make bochs	----install and run"
	@echo "  install_to_machine  ----install sins to you machine(multiboot)"

clean:
	-rm dasm.tmp
	-rm -rf *.o
	-cd arch; make clean
	-cd boot; make clean
	-cd init; make clean
	-cd kernel; make clean
	-cd fs; make clean
	-cd drivers; make clean
	-cd lib; make clean

count: clean
	@echo `expr $(shell find ./ | xargs cat 2>/dev/null| wc -l)	\
		 - $(shell find tools | xargs cat 2>/dev/null| wc -l)` lines


mount:
	if [ ! -e /dev/loop0p1 ];then\
		sudo losetup -P /dev/loop0 tools/disk.img;\
		sudo mount /dev/loop0p1 $(MOUNT_DIR);\
	fi
	@echo 'mount finished!'

umount:
	if [ -e /dev/loop0p1 ];then\
		sudo umount $(MOUNT_DIR);\
		sudo losetup -d /dev/loop0;\
	fi
	@echo 'umount finished!'

install: $(TARGET) mount
	cp $(TARGET) $(MOUNT_DIR)/boot/sins
	sync
	@echo 'install finished!';

backup:	clean 
	tar czvf $(BACKUP_DIR)/sins_$(shell date +%Y_%m_%d_%H_%M_%S).tar.gz ./
	@echo 'backup finished!';

bochs: install
	bochs -q -f .bochs

qemu:	install
	qemu-system-i386 tools/disk.img

install_to_machine: install
	sudo cp boot/sins /boot/sins

dasm:
	$(DASM) $(DASMFLAGS) $(TARGET) > $(DASMFILE)
	vim $(DASMFILE)	
