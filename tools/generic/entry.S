#define SINS_EXEC_MAGIC	0x19879023

.text
.globl _start
_start:
	jmp start
.org 0x04
	.long SINS_EXEC_MAGIC
start:
	push %eax	/* argv */
	push %ebx	/* argc */	
	call main
	add $0x08, %esp
	push %eax
	call exit
