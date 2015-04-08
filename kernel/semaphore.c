#include <sins/semaphore.h>

void down(semaphore_t *s)
{
	unsigned long flags;

	irq_save(flags);
	atomic_inc(&s->count);
	if (atomic_read(&s->count) != 1) {
		wait(&s->wait);
	}
	irq_restore(flags);
}

void up(semaphore_t *s)
{
	unsigned long flags;
	
	irq_save(flags);
	atomic_dec(&s->count);
	wake_up(&s->wait);
	irq_restore(flags);
}
