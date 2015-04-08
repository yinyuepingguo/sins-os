#ifndef _SINS_ATOMIC_H
#define _SINS_ATOMIC_H

typedef struct {
	volatile long counter;
} atomic_t;

#include <asm/atomic.h>

#define ATOMIC_INIT(i)	{ (i) }

static inline long atomic_read(atomic_t *v)
{
	return v->counter;
}

static inline void atomic_set(atomic_t *v, long i)
{
	v->counter = i;
}

#define atomic_inc(v)		arch_atomic_inc(v)
#define atomic_dec(v)		arch_atomic_dec(v)
#define atomic_add(v, i)	arch_atomic_add(v, i)
#define atomic_sub(v, i)	arch_atomic_sub(v, i)

#endif
