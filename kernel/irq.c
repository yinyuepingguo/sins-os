#include <sins/irq.h>
#include <string.h>
#include <sins/kernel.h>

static irq_desc_t irqs[NR_IRQS];


asmlinkage void do_irq(unsigned int irq)
{
	unsigned long flags;

	BUG_ON(irq >= NR_IRQS);

	irq_save(flags);
	if (likely(irqs[irq].handler != NULL))
		irqs[irq].handler();	
	else {
		printk("irq[%d] happened!", irq);
	}
	irq_restore(flags);
}

void enable_irq(unsigned int irq)
{
	unsigned long flags;
	
	irq_save(flags);
	if (irqs[irq].disable != 0)
		irqs[irq].disable--;
	else
		arch_enable_irq(irq);
	irq_restore(flags);
}

void disable_irq(unsigned int irq)
{
	unsigned long flags;

	irq_save(flags);
	if (irqs[irq].disable == 0)
		arch_disable_irq(irq);
	irqs[irq].disable++;
	irq_restore(flags);
}

result_t request_irq(unsigned int irq, irq_handler_t handler, char *name)
{
	result_t res;
	unsigned long flags;

	BUG_ON(irq >= NR_IRQS);

	irq_save(flags);
	if (likely(irqs[irq].handler == NULL && strlen(name) != 0)) {
		irqs[irq].handler = handler;
		irqs[irq].name = name;
		res = SUCCESS;
	} else
		res = -ERROR;
	irq_restore(flags);

	return res;
}

void free_irq(unsigned int irq)
{
	unsigned long flags;

	BUG_ON(irq >= NR_IRQS);

	irq_save(flags);
	irqs[irq].handler = NULL;
	irqs[irq].name = NULL;
	irq_restore(flags);
}
