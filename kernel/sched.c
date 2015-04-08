#include <sins/kernel.h>
#include <sins/sched.h>
#include <sins/irq.h>
#include <sins/fs.h>
#include <sins/syscalls.h>
#include <sins/error.h>
#include <string.h>
#include <sins/signal.h>

#define NR_PID_LISTS	10

static LIST_HEAD(run_list);
static LIST_HEAD(ready_list);
LIST_HEAD(task_list);
static struct list_head pid_lists[NR_PID_LISTS];
static unsigned long pid_start = 0;

static inline void init_pid_lists()
{
	unsigned long i = 0;

	for (i = 0; i != NR_PID_LISTS; ++i)
		INIT_LIST_HEAD(&pid_lists[i]);
}

static inline void add_to_pid_lists(struct task_struct *task)
{
	unsigned long flags;

	irq_save(flags);
	list_add(&task->pid_list, &pid_lists[task->pid % NR_PID_LISTS]);
	irq_restore(flags);
}

static inline void remove_from_pid_lists(struct task_struct *task)
{
	unsigned long flags;

	irq_save(flags);
	list_del(&task->pid_list);
	irq_restore(flags);

}

struct task_struct *get_task_by_pid(unsigned long pid)
{
	struct list_head *list = &pid_lists[pid % NR_PID_LISTS];
	struct task_struct *pos;

	list_for_each_entry(pos, list, pid_list) {
		if (pos->pid == pid)
			return pos;
	}
	return NULL;
}

static unsigned long calc_time_slice(struct task_struct *task)
{
	return 10;
}

/* NOTE:should confirm the zero task get pid[0] and init task get pid[1] */
static unsigned long find_usable_pid()
{
	struct list_head *pos;
	struct task_struct *task;
	unsigned long flags;
	
	irq_save(flags);
	/* bad algorithm, can use bitmap  */
refind:
	list_for_each(pos, &task_list) {
		task = list_entry(pos, struct task_struct, task_list);
		if (unlikely(task->pid == pid_start)) {
			pid_start++;	
			goto refind;
		}
	}
	irq_restore(flags);
	return pid_start++;	
}

static void init_task()
{
	unsigned int i;
	struct task_struct *curr = current;

	curr->state = TASK_RUNNABLE;
	curr->pid = find_usable_pid();
	strncpy(curr->cmd, "idle", TASK_CMD_LENGTH);
	curr->flags = TASK_KTHREAD;

	/* signal */
	curr->signal = 0;
	curr->sig_mask = 0;
	for (i = 0; i != NR_SIGNALS; ++i) {
		curr->sig_fn[i] = NULL;	
	}
	curr->sig_restorer = SIG_RESTORER_NULL;

	/* for scheduler algorithm */	
	curr->time_slice = calc_time_slice(curr);

	/* parent-child relationship */
	curr->exit_code = 0;
	INIT_LIST_HEAD(&curr->wait_exit);
	INIT_LIST_HEAD(&curr->chld_active);
	INIT_LIST_HEAD(&curr->chld_zombie);
	INIT_LIST_HEAD(&curr->child);
	curr->parent = NULL;

	/* vfs specific */
	curr->root = curr->pwd = NULL;

	/* assign kernel page_dir */
	curr->page_dir = kernel_page_dir;

	for (i = 0; i != NR_OPEN; ++i)
		curr->fdt[i] = NULL;

	list_add(&curr->run_list, &run_list);
	list_add(&curr->task_list, &task_list);
	add_to_pid_lists(curr);
}

void sched_init()
{
	init_pid_lists();
	init_task();
}

result_t kernel_thread(const char cmd[],
	int (*pfn)(unsigned long data), unsigned long data)
{
	struct task_struct *task = (struct task_struct *)
		get_free_page(GFP_NORMAL | GFP_DMA);
	unsigned long flags;
	unsigned long i;

	if (task == NULL)
		return -ENOMEM;
	/* init task struct */
	task->state = TASK_RUNNABLE;
	task->pid = find_usable_pid();
	strncpy(task->cmd, cmd, TASK_CMD_LENGTH);
	task->cmd[TASK_CMD_LENGTH-1] = 0;	/* protect */
	task->flags = TASK_KTHREAD;

	/* signal */
	task->signal = 0;
	task->sig_mask = current->sig_mask;
	for (i = 0; i != NR_SIGNALS; ++i) {
		task->sig_fn[i] = NULL;
	}
	task->sig_restorer = SIG_RESTORER_NULL;

	task->time_slice = calc_time_slice(task);

	task->exit_code = 0;
	INIT_LIST_HEAD(&task->wait_exit);
	INIT_LIST_HEAD(&task->chld_active);
	INIT_LIST_HEAD(&task->chld_zombie);
	/* init */
	kthread_context_init(&task->context, pfn, data,
		(unsigned long)task + TASK_SIZE);
	
	/* get a new and empty page dir */
	task->page_dir = copy_empty_page_dir(); 
	if (task->page_dir == NULL) {
		free_page(task);
		return -ENOMEM;
	}
		
	irq_save(flags);
	task->parent = current;

	/* vfs specific */
	task->pwd = current->pwd;
	task->root = current->root;
	if (current->pwd != NULL) {
		dref(current->pwd);
	}
	if (current->root != NULL) {
		dref(current->root);
	}
	for (i = 0; i != NR_OPEN; ++i) {
		task->fdt[i] = current->fdt[i];
		if (task->fdt[i])
			atomic_inc(&task->fdt[i]->ref_count);
	}
	
	list_add(&task->run_list, &ready_list);
	list_add(&task->task_list, &task_list);
	list_add(&task->child, &current->chld_active);
	add_to_pid_lists(task);
	irq_restore(flags);
	return task->pid;
}

void schedule()
{
	struct list_head *pos;
	struct task_struct *task;
	struct task_struct *next;
	unsigned long flags;

	irq_save(flags);
	/* no time slice */
	if (current->time_slice == 0 && current->state == TASK_RUNNABLE) {
		current->time_slice = calc_time_slice(current);
		list_move(&current->run_list, &ready_list);
	} else if (unlikely(current->state == TASK_ZOMBIE)) {
		list_del(&current->run_list);
		BUG_ON(current->parent == NULL);
		list_move(&current->child, &current->parent->chld_zombie);
		wake_up(&current->parent->wait_exit);
	} 

	if (list_empty(&run_list)) {
		/* swap run_list and ready_list */
		list_splice_init(&ready_list, &run_list);
		list_for_each(pos, &run_list) {
			task = list_entry(pos, struct task_struct, run_list);
			task->state = TASK_RUNNABLE;
		}
	}

	BUG_ON(list_empty(&run_list));
	next = list_entry(run_list.next, struct task_struct, run_list);

	if (next != current) {
		switch_pgdir(next->page_dir);
		switch_to(current, next);
	}
	irq_restore(flags);
}

void kthread_release(struct task_struct *task)
{
	struct task_struct *init;
	unsigned long flags;
	struct task_struct *iter;

	BUG_ON(task->state != TASK_DEAD);
	BUG_ON(task->pid == 0);

	irq_save(flags);	
#ifdef DEBUG
	printk("task %d dead!", task->pid);
#endif
	remove_from_pid_lists(task);
	list_del(&task->task_list);
	list_del(&task->child);
	if (!list_empty(&task->chld_active) ||
		!list_empty(&task->chld_zombie)) {
		init = get_task_by_pid(TASK_INIT_PID);
		list_for_each_entry(iter, &task->chld_active, child) {
			iter->parent = init;
		}
		list_for_each_entry(iter, &task->chld_zombie, child) {
			iter->parent = init;
		}
		list_splice(&task->chld_active, &init->chld_active);
		list_splice(&task->chld_zombie, &init->chld_zombie);
		if (!list_empty(&init->chld_zombie))
			wake_up(&init->wait_exit);
	}
	irq_restore(flags);
	if (task->root);
		dput(task->root);
	if (task->pwd)
		dput(task->pwd);
	free_page_dir(task->page_dir);
	free_page(task);
}


/* confirm interrupt is disable */
void sched_tick()
{
	struct task_struct *task = current;

	if (likely(task->time_slice != 0))
		task->time_slice--;
	if (unlikely(task->time_slice == 0)) {
		schedule();
	}
}

void wait(wait_queue_t *queue) {
	unsigned long flags;
	
	irq_save(flags);
	list_del(&current->run_list);
	list_add(&current->run_list, queue);
	current->state = TASK_WAIT;
	irq_restore(flags);
	schedule();
}

void wake_up(wait_queue_t *queue) {
	wait_queue_t *fetch;
	unsigned long flags;

	irq_save(flags);
	if (unlikely(list_empty(queue))) {
		irq_restore(flags);
		return;
	}
	fetch = queue->next;
	list_del(fetch);
	list_add(fetch, &run_list);
	list_entry(fetch, struct task_struct, run_list)->state = TASK_RUNNABLE;
	irq_restore(flags);
}

void wake_up_all(wait_queue_t *queue) {
	wait_queue_t *pos;
	unsigned long flags;

	irq_save(flags);
	list_for_each(pos, queue) {
		list_entry(pos, struct task_struct, run_list)->state
			= TASK_RUNNABLE;
	}
	list_splice_init(queue, &run_list);
	irq_restore(flags);
}

asmlinkage unsigned long sys_getpid()
{
	return current->pid;
}

asmlinkage unsigned long sys_getppid()
{
	if (current->parent == NULL)
		return 0;
	return current->parent->pid;
}
