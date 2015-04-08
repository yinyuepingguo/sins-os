#include <sins/kernel.h>
#include <stdarg.h>
#include <sins/processor.h>
#include <sins/irq.h>
#include <string.h>

void panic(const char *fmt, ...)
{
	static char buf[1024];
	va_list args;
	unsigned long flags;
	
	irq_save(flags);
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	printk("Kernel panic: %s", buf);
	while(1)
		halt();

	/* never run this statement */
	irq_restore(flags);
}
