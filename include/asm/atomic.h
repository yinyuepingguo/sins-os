#ifndef _ASM_ATOMIC_H
#define _ASM_ATOMIC_H

#include <asm/processor.h>

static inline void arch_atomic_inc(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "incl %0"
		: "+m" (v->counter));
}

static inline void arch_atomic_dec(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "decl %0"
		: "+m" (v->counter));
}

static inline void arch_atomic_add(atomic_t *v, long i)
{
	asm volatile(LOCK_PREFIX "addl %1, %0"
		: "+m" (v->counter)
		: "ir" (i));
}

static inline void arch_atomic_sub(atomic_t *v, long i)
{
	asm volatile(LOCK_PREFIX "subl %1, %0"
		: "+m" (v->counter)
		: "ir" (i));
}

#endif
