#ifndef _TYPE_CHECK_H
#define _TYPE_CHECK_H

/*
 * Check at compile time that something is of a particular type.
 */
#define TYPE_CHECK(type, x)	\
	({	\
		type __dummy;	\
		typeof(x) __dummy2;	\
		(void)(&__dummy == &__dummy2);	\
		1;	\
	})

#define TYPE_SAME(x, y)	\
	({	\
		typeof(x) __dummy;	\
		typeof(y) __dummy2;	\
		(void)(&__dummy == &__dummy2);	\
		1;	\
	})

#define TYPE_CHECK_FN(type, function)	\
	({	\
		typeof(type) __tmp = function;	\
		(void)__tmp;	\
	})

#endif
