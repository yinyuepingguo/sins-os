#include <sins/notifier.h>
#include <sins/irq.h>
#include <sins/kernel.h>

void init_notifier_chain(struct notifier_chain *chain)
{
	INIT_LIST_HEAD(&chain->list);
}

void add_notifier(struct notifier_handler *handler, struct notifier_chain *chain)
{
	unsigned long flags;

	BUG_ON(handler->function == NULL);

	irq_save(flags);
	list_add(&handler->list, &chain->list);
	irq_restore(flags);
}
void del_notifier(struct notifier_handler *handler)
{
	unsigned long flags;
	
	irq_save(flags);
	list_del_init(&handler->list);
	irq_restore(flags);
}
void do_notifier(struct notifier_chain *chain, unsigned long data)
{
	struct list_head *pos, *tmp;
	struct notifier_handler *handler;
	
	list_for_each_safe(pos, tmp, &chain->list) {
		handler = list_entry(pos, struct notifier_handler, list);
		BUG_ON(handler->function == NULL);
		handler->function(data);
	}
}

