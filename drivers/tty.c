#include "console.h"
#include "keyboard.h"
#include <sins/fs.h>
#include <sins/kernel.h>
#include <sins/init.h>
#include <sins/semaphore.h>
#include <ctype.h>
#include <list.h>

struct tty_char {
	byte data;
	struct list_head list;
};

static result_t push_char(byte b, struct file *filp)
{
	struct tty_char *c;
	c = kmalloc(sizeof(struct tty_char));
	if (!c) 
		return -ENOMEM;
	c->data = b;
	list_add_tail(&c->list, (struct list_head *)filp->private_data);
	return SUCCESS;
}

static result_t pop_char(int back, struct file *filp) {
	struct list_head *head;
	struct tty_char *c;
	result_t ret = 0;

	head = (struct list_head *)filp->private_data;

	if (list_empty(head))
		return -ERROR;
	if (back)
		head = head->prev;
	else
		head = head->next;
	c = list_entry(head, struct tty_char, list);
	list_del(&c->list);
	ret = c->data;
	kfree(c);
	return ret;
}

static int tty_isbreak(struct kbd_event *event)
{
	return event->kv.val[0] == '\r';
}

static result_t tty_open(struct inode *inode, struct file *filp)
{
	filp->private_data = kmalloc(sizeof(struct list_head));
	if (!filp->private_data)
		return -ENOMEM;
	INIT_LIST_HEAD((struct list_head *)filp->private_data);
	return SUCCESS;
}

static result_t tty_close(struct inode *inode, struct file *filp)
{
	struct list_head *pos, *tmp;
	struct tty_char *c;
	list_for_each_safe(pos, tmp, (struct list_head*)filp->private_data) {
		c = list_entry(pos, struct tty_char, list);
		list_del(pos);
		kfree(c);
	}
	kfree(filp->private_data);
	return SUCCESS;
}

static ssize_t tty_write
	(struct file *filp, const byte __user *buf, size_t size, loff_t *pos)
{
	con_write((const char *)buf, size);
	return size;
}

static ssize_t tty_read
	(struct file *filp, byte __user *buf, size_t size, loff_t *pos)
{
	static WAIT_QUEUE(wait_read);
	static SEMAPHORE(read_sem);
	struct kbd_event event;
	ssize_t read_count = 0;
	result_t res;

	down(&read_sem);
	while (1) {
		keyboard_read(&event);

		if (tty_isbreak(&event)) {
			printk("\n");
			break;
		} else if (isprint(event.kv.val[0])) {
			if (push_char(event.kv.val[0], filp) >= 0) {
				printk("%c", event.kv.val[0]);
			} else {
				break;
			}
		} else if (event.kv.val[0] == '\b') {
			if (pop_char(1, filp) >= 0) {
				printk("\b \b");
			}
		} else if (event.kv.val[0] == '\t') {
			if (push_char(' ', filp) >= 0) {
				printk(" ");
			} else {
				break;
			}
		}
	}
	for (; read_count < size; read_count++) {
		if ((res = pop_char(0, filp)) >= 0) {
			buf[read_count] = (byte)res;
		} else {
			buf[read_count] = 0;
			break;
		}
	}
	up(&read_sem);
	return read_count; 
}

static struct file_operations tty_fops = {
	.open	= tty_open,
	.read	= tty_read,
	.write	= tty_write,
	.close	= tty_close
};

static result_t tty_setup()
{
	register_chrdev(DEV_TTY, "tty", &tty_fops);
	return SUCCESS;
}

device_initcall(tty_setup);
