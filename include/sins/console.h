#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <types.h>
#include <list.h>

#define CON_ENABLE 0x01
#define CON_BOOT 0x02

struct console {
	void (*write)(const char *str, size_t count);
	result_t (*release)();
	unsigned long flags;
	struct list_head list;
};

extern result_t register_console(struct console *);
extern void unregister_console(struct console *);
extern void console_write(const char *buf, size_t count);
extern void console_init();

#endif
