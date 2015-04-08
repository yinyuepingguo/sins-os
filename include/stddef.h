#ifndef _STDDEF_H
#define _STDDEF_H

/* Include definitions usually used(everyone can use) */
#include <types.h>
#include <sins/error.h>

#define NULL ((void *) 0)

enum {
	false = 0,
	true = 1
};

#define USHORT_MAX	((u16)(~0U))
#define SHORT_MAX	((s16)(USHORT_MAX>>1))
#define SHORT_MIN	(-SHORT_MAX - 1)
#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member)	({			\
		const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
		(type *)((char *)__mptr - offsetof(type,member));	\
	})

#define array_size(arr)	(sizeof(arr) / sizeof(arr[0]))

#define likely(x)	__builtin_expect(!!(x), true)
#define unlikely(x)	__builtin_expect(!!(x), false)

/* Mask deprecated if function don't in future */
#ifndef __deprecated
# define __deprecated		/* unimplemented */
#endif

#endif
