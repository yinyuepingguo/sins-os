#include <sins/kernel.h>
#include <sins/init.h>
#include <sins/irq.h>
#include <asm/io.h>
#include <sins/processor.h>
#include <sins/timer.h>
#include <sins/sched.h>

#define LATCH (1193180/HZ)

static void timer_handler(void)
{
	do_timer();
	sched_tick();
}

static result_t timer_setup()
{
	result_t res;

	res = request_irq(IRQ_TIMER, timer_handler, "timer");
	if (FAILED(res))
		return -ERROR;

	outb_p(0x36, 0x43);
	outb_p(LATCH & 0xff, 0x40);
	outb_p(LATCH >> 8, 0x40);	
	enable_irq(IRQ_TIMER);

	return res;
}

archdone_initcall(timer_setup);
