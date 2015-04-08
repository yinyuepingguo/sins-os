#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#ifndef __ASSEMBLY__

#include <bitops.h>
#include <asm/io.h>

#endif

#define IRQ_TIMER	0
#define IRQ_KEYBOARD	1
#define IRQ_CASCADE	2
#define IRQ_ETHER	3
#define IRQ_SECONDARY	3
#define IRQ_RS232	4
#define IRQ_XT_WINI	5
#define IRQ_FLOPPY	6
#define IRQ_PRINTER	7
#define IRQ_FPU 	13
#define IRQ_AT_WINI	14

#define ARCH_NR_IRQS	16

#ifndef __ASSEMBLY__

#define arch_irq_enable()	asm volatile("sti":::)
#define arch_irq_disable()	asm volatile("cli":::)

#define arch_irq_save(flags)	asm volatile(	\
		"pushf ;pop %0"	\
		: "=rm" (flags)	\
		: /* no input */\
		: "memory")
#define arch_irq_restore(flags) asm volatile(	\
		"push %0 ;popf"	\
		: /* no output */\
		: "g" (flags)	\
		: "memory", "cc")

static inline void arch_enable_irq(unsigned int irq)
{
	unsigned long port = 0x21;
	
	if (irq >= 8)
	{
		port = 0xA1;
		irq -= 8;
	}
	outb_p(inb_p(port) & ~BIT(irq), port);
}

static inline void arch_disable_irq(unsigned int irq)
{
	unsigned long port = 0x21;
	
	if (irq >= 8)
	{
		port = 0xA1;
		irq -= 8;
	}
	outb_p(inb_p(port) & BIT(irq), port);
}

#endif /* not def __ASSEMBLY__ */

#endif
