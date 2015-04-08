#include <sins/kernel.h>
#include <sins/sched.h>
#include <sins/irq.h>
#include <sins/error.h>
#include <sins/syscalls.h>
#include <sins/fs.h>
#include <sins/signal.h>

asmlinkage void sys_exit(result_t code)
{
	struct task_struct *curr = current;
	unsigned long flags;
	unsigned long i;

	/* doing...context switch and remove current context? */
	BUG_ON(curr->pid == 0);

	for (i = 0; i != NR_OPEN; ++i) {
		if (curr->fdt[i])
			sys_close(i);
	}

	irq_save(flags);
	curr->state = TASK_ZOMBIE;
	curr->exit_code = code;
	/* if I'm a usermode process, notify my parent(kernel mode no signal */
	if (!is_kthread(curr))
		sys_kill(curr->parent->pid, SIGCHLD);
	/* schedule once */
	schedule();
	irq_restore(flags);
	BUG("scheduler--exit error!");
}



asmlinkage result_t sys_wait_pid(unsigned long pid)
{
	struct task_struct *task = get_task_by_pid(pid);
	result_t exit_code;
	
	if (task == NULL || task->parent != current)
		return -ERROR;

	wait_event(&current->wait_exit,
		task->state == TASK_ZOMBIE);

	task->state = TASK_DEAD;
	exit_code = task->exit_code;

	kthread_release(task);
	return exit_code;
}

asmlinkage result_t sys_wait_chld(unsigned long *pid)
{
	struct task_struct *task;
	result_t exit_code;
	unsigned long flags;

	wait_event(&current->wait_exit,
		!list_empty(&current->chld_zombie));

	irq_save(flags);
	task = list_first_entry(&current->chld_zombie,
		struct task_struct, child);	
	irq_restore(flags);

	BUG_ON(task->pid == 0);
	task->state = TASK_DEAD;
	if (pid != NULL)
		*pid = task->pid;
	exit_code = task->exit_code;

	kthread_release(task);
	return exit_code;
}


 
