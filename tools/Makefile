#			SINS
#	This is Makefile for whole kernel.
#	You can type 'make help' to get some usage.

SINS_PATH	= /var/sins
INSTALL_PATH	= /var/sins.disk/bin/
GENERIC_PATH	= $(SINS_PATH)/tools/generic

export CC	= gcc
export AS	= gcc
export LD	= ld
export DASM	= objdump
export CFLAGS	= -I $(SINS_PATH)/include -nostdinc -fno-builtin -Wall -O0
export ASMFLAGS = -I $(SINS_PATH)/include -nostdinc -fno-builtin -Wall -D__ASSEMBLY__
export DASMFLAGS = -D
export DASMFILE	= dasm.tmp
export LDFLAGS	= -nostdlib -nostdinc

objs-basic	= $(GENERIC_PATH)/entry.o $(SINS_PATH)/lib/lib.o $(GENERIC_PATH)/libc.o
objs-template	= template/main.o
objs-sh		= sh/main.o
objs-echo	= echo/main.o
objs-ls		= ls/main.o
objs-cat	= cat/main.o
objs-wc		= wc/main.o
objs-ps		= ps/main.o
objs-debug	= debug/main.o
objs-hexdump	= hexdump/main.o
objs-date	= date/main.o


all: sh echo ls cat wc ps debug hexdump date

template: $(objs-basic) $(objs-template)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

sh: $(objs-basic) $(objs-sh)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

echo: $(objs-basic) $(objs-echo)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

ls: $(objs-basic) $(objs-ls)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

cat: $(objs-basic) $(objs-cat)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

wc: $(objs-basic) $(objs-wc)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

ps: $(objs-basic) $(objs-ps)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

debug: $(objs-basic) $(objs-debug)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

hexdump: $(objs-basic) $(objs-hexdump)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

date: $(objs-basic) $(objs-date)
	make link.lds
	ld $(LDFLAGS) -T $(GENERIC_PATH)/link.lds -o $@/$@ $^
	objcopy $@/$@ -O binary

link.lds:
	gcc -P  $(ASMFLAGS) $(GENERIC_PATH)/link.lds.S -E -o $(GENERIC_PATH)/link.lds



$(SINS_PATH)/lib/lib.o:
	cd $(SINS_PATH); make lib/lib.o

.c.o:
	$(CC) $(CFLAGS) -o $*.o -c $<
.S.o:
	$(AS) $(ASMFLAGS) -o $*.o -c $<

.PHONY: clean dasm install link.lds

clean:
	-rm -rf *.o
	-rm -rf $(GENERIC_PATH)/*.o
	-cd sh;rm -rf *.o
	-cd echo;rm -rf *.o
	-cd ls;rm -rf *.o
	-cd cat;rm -rf *.o
	-cd wc;rm -rf *.o
	-cd ps;rm -rf *.o
	-cd debug;rm -rf *.o
	-cd hexdump;rm -rf *.o
	-cd date;rm -rf *.o
	-cd template;rm -rf *.o
	-rm $(GENERIC_PATH)/link.lds
	-rm dasm.tmp

install: $(TARGET)
	cp sh/sh echo/echo ls/ls cat/cat wc/wc ps/ps debug/debug	\
		hexdump/hexdump	date/date $(INSTALL_PATH)
