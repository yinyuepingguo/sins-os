#include <sins/kernel.h>
#include <sins/timer.h>
#include <list.h>
#include <sins/irq.h>
#include <div64.h>

#define NR_TIMER_LISTS 60 

volatile u64 jiffies = 0;

struct list_head timer_lists [NR_TIMER_LISTS];

asmlinkage void do_timer()
{
	struct list_head *pos;
	struct list_head *tmp;
	struct timer_handler *handler;
	u64 now = jiffies;
	u64 now_tmp = now;
	unsigned long list_nr;

	list_nr = do_div64(now_tmp, NR_TIMER_LISTS);
	
	list_for_each_safe(pos, tmp, &timer_lists[list_nr]) {
		handler = list_entry(pos,struct timer_handler, list);

		BUG_ON(now > handler->jiffies);
		if (now == handler->jiffies) {
			handler->function(handler->data);
			if (handler->times != -1) {
				handler->times--;
			}
			del_timer(handler);
			if (handler->times != 0)
				add_timer(handler);
		} 
	}
	jiffies++;
}

void add_timer(struct timer_handler *handler)
{
	unsigned long flags;
	u64 tmp;

	BUG_ON(handler->expires == 0 && handler->times == -1);
	BUG_ON(handler->function == NULL);

	if (handler->times == 0)
		return; /* do nothing*/

	irq_save(flags);
	if (handler->expires == 0) {
		handler->function(handler->data);
		INIT_LIST_HEAD(&handler->list);
	} else {
		handler->jiffies = handler->expires + jiffies;
		tmp = handler->jiffies;
		list_add(&handler->list,
			&timer_lists[do_div64(tmp, NR_TIMER_LISTS)]);
	}
	irq_restore(flags);
}

void del_timer(struct timer_handler *handler)
{
	unsigned long flags;

	irq_save(flags);
	list_del_init(&handler->list);
	irq_restore(flags);
}

void timer_init()
{
	unsigned int i = 0;

	jiffies = 0;
	for (i = 0; i != NR_TIMER_LISTS; ++i)
		INIT_LIST_HEAD(&timer_lists[i]);
}
