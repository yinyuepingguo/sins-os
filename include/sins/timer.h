#ifndef _SINS_TIMER_H
#define _SINS_TIMER_H

#include <list.h>
#include <linkage.h>
#include <sins/kernel.h>
#include <asm/timer.h>

#define HZ ARCH_HZ
#define TIMER_INFINITE	-1
#define TIMER_ONCE	1

typedef struct timer_handler {
	struct list_head list;
	unsigned long expires;
	u64 jiffies;
	void (*function)(unsigned long data);
	unsigned long data;
	/* call times. -1 presents forever. */
	long times;
} timer_t;

extern volatile u64 jiffies;

extern void timer_init();
extern asmlinkage void do_timer();
extern void add_timer(struct timer_handler *handler);
extern void del_timer(struct timer_handler *handler);

#endif
