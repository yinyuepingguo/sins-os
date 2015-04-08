#include <sins/fs.h>
#include <sins/kernel.h>
#include <sins/init.h>
#include <ctype.h>
#include <list.h>
#include <sins/sched.h>
#include <string.h>

extern struct list_head task_list;
static ssize_t proc_read
	(struct file *filp, byte __user *buf, size_t size, loff_t *pos);

static struct file_operations proc_fops = {
	.read	= proc_read,
};

/* time has spend too long after you got these information.So... */
static ssize_t proc_read
	(struct file *filp, byte __user *buf, size_t size, loff_t *pos)
{
	struct task_struct *task;
	unsigned long flags;
	size_t max_count = size / sizeof(struct task_info);
	size_t count = 0;
	struct task_info *info = (struct task_info *)buf;
	unsigned long i, files;

	irq_save(flags);
	list_for_each_entry (task, &task_list, task_list) {
		if (count != max_count) {
			/* initilize task_info struct */
			info->state = task->state;
			info->pid = task->pid;
			strncpy(info->cmd, task->cmd, TASK_CMD_LENGTH);
			info->time_slice = task->time_slice;
			info->flags = task->flags;
			if (task->parent == NULL) {
				info->ppid = -1;
				strcpy(info->pcmd, "");
			} else {
				info->ppid = task->parent->pid;
				strcpy(info->pcmd, task->parent->cmd);
			}
			files = 0;
			for (i = 0; i != NR_OPEN; ++i) {
				if (task->fdt[i] != NULL)
					++files;
			}
			info->files = files;
			if (task->root) {
				strncpy(info->root,
					task->root->name, MAX_NAME_LEN);
				if (strnlen(info->root, MAX_NAME_LEN) == 0) {
					strcpy(info->root, "/");
				}
			} else
				strcpy(info->root, "/");
			if (task->pwd) {
				strncpy(info->pwd,
					task->pwd->name, MAX_NAME_LEN);
				if (strnlen(info->pwd, MAX_NAME_LEN) == 0) {
					strcpy(info->pwd, "/");
				}
			} else
				strcpy(info->pwd, "/");
		} else {
			break;
		}
		++info;
		++count;
	}
	irq_restore(flags);

	return count * sizeof(struct task_info);
}


static result_t proc_setup()
{
	register_chrdev(DEV_PROC, "proc", &proc_fops);
	return SUCCESS;
}

device_initcall(proc_setup);
