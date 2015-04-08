#ifndef _ASM_SCHED_H
#define _ASM_SCHED_H

#include <types.h>
#include <sins/page.h>
#include <sins/processor.h>

struct task_struct;

struct arch_task_context {
	u32	esp;
	u32	eip;	
};

static inline struct task_struct *arch_get_current()
{
	struct task_struct *current;
	__asm__("movl %%esp, %0;":"=r" (current));
	current = (struct task_struct *)
		PAGE_ALIGN(((unsigned long)current - PAGE_SIZE));
	return current;
}

#define arch_switch_to(prev,next) do {                    \
	struct task_struct *_prev = prev;	\
	struct task_struct *_next = next;	\
	write_esp0((unsigned long)next+PAGE_SIZE);	\
	asm volatile(	\
		"pusha\n\t"	\
		"pushf\n\t"	\
		"movl %%esp,%0\n\t"    /* save ESP */        \
		"movl %2,%%esp\n\t"    /* restore ESP */    \
		"movl $1f,%1\n\t"        /* save EIP */        \
		"pushl %3\n\t"        /* restore EIP */    \
		"ret\n\t"	\
		"1:\t"                        \
		"popf\n\t"	\
		"popa\n\t"	\
		:"=m" (_prev->context.esp),"=m" (_prev->context.eip)    \
		:"m" (_next->context.esp),"m" (_next->context.eip));    \
} while (0)

extern void arch_kthread_context_init(struct arch_task_context *context,
	int (*pfn)(unsigned long data), unsigned long data,
		unsigned long stack);

#endif
