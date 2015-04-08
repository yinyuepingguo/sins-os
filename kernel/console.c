#include <sins/kernel.h>
#include <sins/console.h>
#include <stddef.h>
#include <sins/init.h>

static LIST_HEAD(console_list);

result_t register_console(struct console *con)
{
	if (con->write == NULL)
		return -ERROR;

	list_add(&con->list, &console_list);
	return SUCCESS;
}

void unregister_console(struct console *con)
{
	if (con->release != NULL)
		con->release();
	list_del(&con->list);
}

void console_write(const char *buf, size_t count)
{
	struct console *con;

	list_for_each_entry(con, &console_list, list) {
		if (con->flags & CON_ENABLE)
			con->write(buf, count);
	}
}

void console_init()
{
	struct list_head *pos;
	struct list_head *tmp;
	struct console *con;

	list_for_each_safe(pos, tmp, &console_list) {
		con = list_entry(pos,struct console, list);
		if (con->flags & CON_BOOT)
			unregister_console(con);
	}
}
