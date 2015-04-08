#include <sins/kernel.h>
#include <string.h>
#include <stdarg.h>
#include <sins/console.h>
#include <sins/irq.h>

int printk(const char *fmt, ...)
{
	static char buf[1024];
	va_list args;
	int i;
	unsigned long flags;

	irq_save(flags);
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	console_write(buf, i);
	irq_restore(flags);
	return i;
}
