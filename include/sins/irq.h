#ifndef _SINS_IRQ_H
#define _SINS_IRQ_H

#include <asm/irq.h>

#ifndef __ASSEMBLY__

#include <type_check.h>
#include <linkage.h>
#include <types.h>

#endif

#define NR_IRQS ARCH_NR_IRQS

/* 
 *                                +---enable_irq(0)         ----irq_handler0
 *            	                  |---enable_irq(1)         ----irq_handler1
 * irq_enable()----do_irq(irq)----+---enable_irq(2)         ----NULL
 *                                |---enable_irq(...)       ----irq_handler...
 *                                +---enable_irq(NR_IRQS-1) ----irq_handler...
 */

/* enable interrupt */
#define irq_enable()	arch_irq_enable()
/* disable interrupt */
#define irq_disable()	arch_irq_disable()

/* save irq state and disable it */
#define irq_save(flags)		\
	{	\
		TYPE_CHECK(unsigned long, flags);	\
		arch_irq_save(flags);	\
		arch_irq_disable();	\
	}

/* restore irq state */
#define irq_restore(flags)	\
	{	\
		TYPE_CHECK(unsigned long, flags);	\
		arch_irq_restore(flags);	\
	}

#ifndef __ASSEMBLY__

typedef void (*irq_handler_t)(void);

typedef struct {
	char *name;
	irq_handler_t handler;
	unsigned int disable;
} irq_desc_t;

extern asmlinkage void do_irq(unsigned int irq);
extern result_t request_irq(unsigned int irq,
		irq_handler_t handler, char *name);
extern void free_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
extern void disable_irq(unsigned int irq);

#endif /* not def __ASSEMBLY__ */

#endif
