#ifndef _SINS_SEMPHERE_H
#define _SINS_SEMPHERE_H

#include <sins/atomic.h>
#include <sins/sched.h>
#include <sins/irq.h>

typedef struct {
	atomic_t count;
	wait_queue_t wait;	
	
} semaphore_t;

#define SEMAPHORE(name) semaphore_t name = {{0}, WAIT_QUEUE_INIT(name.wait)}

extern void down(semaphore_t *s);
extern void up(semaphore_t *s);

#endif
