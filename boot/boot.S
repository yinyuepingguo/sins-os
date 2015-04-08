/* boot.S - 引导内核 */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#define ASM_FILE
#include <boot/multiboot.h>

#define PAGE_SIZE 0x1000
#define PAGE_OFFSET 0xC0000000

.section ".boot", "ax"
.globl  _start
page_dir:
_start:
        jmp multiboot_entry

        .align  4
multiboot_header:
        /* magic */
        .long   MULTIBOOT_HEADER_MAGIC
        /* flags */
        .long   MULTIBOOT_HEADER_FLAGS
        /* checksum */
        .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	/* header_addr */
	.long 0
	/* load_addr */
	.long 0
	/* load_end_addr */
	.long 0
	/* bss_end_addr */
	.long 0
	/* entry_addr */
	.long 0
	/* mode_type */
	.long 0
	/* width */
	.long 0 
	/* height */
	.long 0
	/* depth */
	.long 0

multiboot_entry:
        /* 初始化堆栈指针。*/
	movl	$__load, %esp
        movl    $(__init_task_end - PAGE_OFFSET), %esp
	jmp after_page_tables
error:   hlt
        jmp     error

.org 0x1000 
pg0:

.org 0x2000
pg1:

.org 0x4000
after_page_tables:
        /* 重置 EFLAGS。 */
        pushl   $0
        popf
        /* 将指向 Multiboot 信息结构的指针入栈。 */
        pushl   %ebx
        /* 将魔数入栈。 */
        pushl   %eax

	/* setup idt */
	lea ignore_int,%edx
        movl $0x00080000,%eax
        movw %dx,%ax            /* selector = 0x0008 = cs */
        movw $0x8E00,%dx        /* interrupt gate - dpl=0, present */
        lea _idt,%edi
	mov $256,%ecx
rp_sidt:
        movl %eax,(%edi)
        movl %edx,4(%edi)
        addl $8,%edi
        dec %ecx
        jne rp_sidt
        lidt idt_descr
	
	/* setup_gdt */
	lgdt gdt_descr
	movl $0x10, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs	
	
	movl $1024*3, %ecx
        xorl %eax, %eax
        movl $page_dir, %edi              /* pg_dir is at 0x000 */
        cld;rep;stosl
        movl $pg0+7, page_dir             /* set present bit/user r/w */
        movl $pg1+7, page_dir+4*1           /*  --------- " " --------- */
	movl $pg0+7, page_dir+768*4
	movl $pg1+7, page_dir+769*4
        movl $pg1+4092, %edi
        movl $0x7ff003, %eax             /*  8Mb - 4096 + 7 (r/w user,p) */
        std 
1:      stosl                   /* fill pages backwards - more efficient :-) */
        subl $0x1000, %eax
        jge 1b 
	movl $page_dir, %eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0

	/* esp point to page mode. */	
	add $PAGE_OFFSET, %esp
	/* param has been pushed to stack */
	movl $1f, %eax
	addl $PAGE_OFFSET, %eax
	push %eax
	jmp start_kernel
1:
	addl $8, %esp
	
	/* should never return.but can debug */
	push $dbg_msg + PAGE_OFFSET
	movl $printk, %eax
	call *%eax
	addl $4, %esp
1:
	hlt
	jmp 1b

dbg_msg:
	.asciz "halted!"

.align 2
ignore_int:
        incb 0xb8000+160                # put something on the screen
        movb $2,0xb8000+161             # so that we know something
        iret   


.align 2
.word 0
idt_descr:
	.word 256*8-1
	.long _idt
.align 2
.word 0
gdt_descr:
	.word 256*8-1
	.long _gdt

.align 8
_idt:	.fill 256,8,0

_gdt:	.quad 0x0000000000000000
	.quad 0x00cf9b000000ffff
	.quad 0x00cf93000000ffff
	.quad 0x0000000000000000
	.fill 252,8,0
/* confirm .boot section's size is 0x5000*/
