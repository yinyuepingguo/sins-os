#ifndef _DIV64_H
#define _DIV64_H

/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * The semantics of do_div64() are:
 *
 * u32 do_div64(u64 *n, u32 base)
 * {
 * 	u32 remainder = *n % base;
 * 	*n = *n / base;
 * 	return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
 */

#include <stddef.h>
#include <type_check.h>

#if BITS_PER_LONG == 64

# define do_div64(n,base) ({					\
	u32 __base = (base);				\
	u32 __rem;						\
	__rem = ((u64)(n)) % __base;			\
	(n) = ((u64)(n)) / __base;				\
	__rem;							\
 })

/**
 * div_u64_rem - unsigned 64bit divide with 32bit divisor with remainder
 *
 * This is commonly provided by 32bit archs to provide an optimized 64bit
 * divide.
 */
static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/**
 * div_s64_rem - signed 64bit divide with 32bit divisor with remainder
 */
static inline s64 div_s64_rem(s64 dividend, s32 divisor, s32 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/**
 * div64_u64 - unsigned 64bit divide with 64bit divisor
 */
static inline u64 div64_u64(u64 dividend, u64 divisor)
{
	return dividend / divisor;
}


#elif BITS_PER_LONG == 32

extern u32 __div64_32(u64 *dividend, u32 divisor);

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
# define do_div64(n,base) ({				\
	u32 __base = (base);				\
	u32 __rem;					\
	TYPE_CHECK(u64, n);				\
	if (likely(((n) >> 32) == 0)) {			\
		__rem = (u32)(n) % __base;		\
		(n) = (u32)(n) / __base;		\
	} else 						\
		__rem = __div64_32(&(n), __base);	\
	__rem;						\
 })

static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder)
{
	TYPE_CHECK(u64, dividend);
	*remainder = do_div64(dividend, divisor);
	return dividend;
}

extern s64 div_s64_rem(s64 dividend, s32 divisor, s32 *remainder);
extern u64 div64_u64(u64 dividend, u64 divisor);

/**
 * div_u64 - unsigned 64bit divide with 32bit divisor
 *
 * This is the most common 64bit divide and should be used if possible,
 * as many 32bit archs can optimize this variant better than a full 64bit
 * divide.
 */
static inline u64 div_u64(u64 dividend, u32 divisor)
{
	u32 remainder;
	return div_u64_rem(dividend, divisor, &remainder);
}

/**
 * div_s64 - signed 64bit divide with 32bit divisor
 */
static inline s64 div_s64(s64 dividend, s32 divisor)
{
	s32 remainder;
	return div_s64_rem(dividend, divisor, &remainder);
}

extern u32 iter_div_u64_rem(u64 dividend, u32 divisor, u64 *remainder);

static inline
u32 __iter_div_u64_rem(u64 dividend, u32 divisor, u64 *remainder)
{
	u32 ret = 0;

	while (dividend >= divisor) {
		/* The following asm() prevents the compiler from
		   optimising this loop into a modulo operation.  */
		asm("" : "+rm"(dividend));

		dividend -= divisor;
		ret++;
	}

	*remainder = dividend;

	return ret;
}

#else /* BITS_PER_LONG == ?? */

# error do_div64() does not yet support the C64

#endif /* BITS_PER_LONG */

#endif
