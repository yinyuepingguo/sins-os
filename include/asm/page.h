#ifndef _ASM_PAGE_H
#define _ASM_PAGE_H

#define arch_switch_pgdir(pgdir)	\
	asm volatile("movl %0, %%cr3"::"r" (pgdir))

#endif

