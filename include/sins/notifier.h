#ifndef _SINS_NOTIFIER_H
#define _SINS_NOTIFIER_H

#include <list.h>

struct notifier_handler {
	struct list_head list;
	void (*function)(unsigned long data);
};

struct notifier_chain {
	struct list_head list;
};

extern void init_notifer_chain(struct notifier_chain *chain);
extern void add_notifier(struct notifier_handler *handler,
	struct notifier_chain *chain);
extern void del_notifier(struct notifier_handler *handler);
extern void do_notifier(struct notifier_chain *chain, unsigned long data);

#endif
