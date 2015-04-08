#ifndef _SINS_SCHED_H
#define _SINS_SCHED_H

#include <asm/sched.h>
#include <sins/mm.h>
#include <list.h>
#include <linkage.h>
#include <types.h>
#include <sins/irq.h>
#include <sins/fs.h>
#include <sins/signal.h>

#define TASK_RUNNABLE	0
#define TASK_WAIT	1
#define TASK_ZOMBIE	2
#define TASK_DEAD	3

/* task flags */
#define TASK_KTHREAD	0x01

#define WAIT_QUEUE(name) LIST_HEAD(name)
#define INIT_WAIT_QUEUE(name) INIT_LIST_HEAD(name)
#define WAIT_QUEUE_INIT(name) LIST_HEAD_INIT(name)

typedef struct list_head wait_queue_t;

#define TASK_SIZE PAGE_SIZE
#define TASK_CMD_LENGTH	20

#define task_context arch_task_context
#define switch_to(prev, next) arch_switch_to(prev, next)

struct task_struct {
	/* process state */
	long state;
	unsigned long pid;
	char cmd[TASK_CMD_LENGTH];
	unsigned long flags;

	/* signal */
	unsigned long signal;
	unsigned long sig_mask;
	sig_handler_t sig_fn[NR_SIGNALS];
	unsigned long sig_restorer;

	/* exit code */
	result_t exit_code;
	/* I wait somebody */
	wait_queue_t wait_exit;

	/* used for schedule */
	unsigned long time_slice;
	struct list_head run_list;
	struct list_head task_list;

	/* used for parent-children relationship */
	struct list_head chld_active;
	struct list_head chld_zombie;
	/* node in my parent's list */
	struct list_head child;
	struct task_struct *parent;

	/* used for pid hash */
	struct list_head pid_list;
	
	/* files struct */
	struct file *fdt[NR_OPEN];
	
	struct dentry *root, *pwd;
	
	pgd_t *page_dir;

	/* cpu specfic */
	struct task_context context;
};

union task_union {
	struct task_struct task;
	unsigned long stack[TASK_SIZE/sizeof(unsigned long)];
};

struct task_info {
	long state;
	unsigned long pid;
	unsigned long flags;
	char cmd[TASK_CMD_LENGTH];
	unsigned long time_slice;
	unsigned long ppid;
	char pcmd[TASK_CMD_LENGTH];
	/* num of files opened */
	unsigned long files;	
	char root[MAX_NAME_LEN];
	char pwd[MAX_NAME_LEN];
};

#define get_current()	arch_get_current()
#define current get_current()

#define kthread_context_init(context, pfn, data, stack)	\
	arch_kthread_context_init(context, pfn, data, stack)

extern struct list_head task_list;

extern void sched_init();
extern void schedule();
extern void sched_tick();
extern void wait(wait_queue_t *queue);
extern void wake_up(wait_queue_t *queue);
extern void wake_up_all(wait_queue_t *queue);
extern result_t kernel_thread(const char cmd[],
	int (*pfn)(unsigned long data), unsigned long data);
extern struct task_struct *get_task_by_pid(unsigned long pid);
extern void kthread_release(struct task_struct *task);

#define is_kthread(task) ((task)->flags & TASK_KTHREAD)


#define TASK_INIT_PID 1

#define wait_event(queue, event)	\
	({		\
		unsigned long flags;\
		while(1) {	\
			irq_save(flags);	\
			if (!(event))	\
				wait(queue);	\
			else	\
				break;	\
			irq_restore(flags);	\
		}	\
	})

#endif
