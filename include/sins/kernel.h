#ifndef _SINS_KERNEL_H
#define _SINS_KERNEL_H

/* Include mostly required headers */
#include <types.h>
#include <stddef.h>
#include <linkage.h>
#include <sins/error.h>

/* Include definitions usually used by kernel and modules */

/* 
*	use DEBUG macro to output some hidden
*	info created by developer.
*/
//#define DEBUG


/*
*	You can choose to close BUG check.
*	BUT I don't advise to do this.
*	BUG check use little cpu circles.
*/
//#define NOBUG
//#define NOWARN

#define FAILED(expr) ((expr) < 0)

#define __user
#define __kernel
#define __iomem

#define __used __attribute__((__used__))
#define __init __used

#ifndef NOBUG
# define BUG(reason) do {	\
		panic("BUG on %s(%s:%d), %s",	\
			__FUNCTION__,	\
			__FILE__,	\
			__LINE__,	\
			(reason)); \
	} while (0)
#else
# define BUG(reason) do {} while (0)
#endif

#ifndef NOWARN
# define WARN(reason) do {	\
		printk("WARN on %s(%s:%d), %s\n",	\
			__FUNCTION__,	\
			__FILE__,	\
			__LINE__,	\
			(reason)); \
	} while (0)
#else
# define WARN(reason) do {} while (0)
#endif

#ifndef NOBUG
# define BUG_ON(condition) do {	\
		if (unlikely(condition)) { \
			BUG(#condition" is happened!");	\
		} 	\
	} while (0)
#else
# define BUG_ON(condition) (condition)
#endif

#ifndef NOWARN
# define WARN_ON(condition) do { \
		if (unlikely(condition))	{ \
			WARN(#condition" is happened!"); \
		}	\
	} while (0)
#else
# define WARN_ON(condition) (condition)
#endif

extern int printk(const char *fmt, ...);
extern void panic(const char *fmt, ...);

#define SINS_EXEC_MAGIC		0x19879023UL

#endif
