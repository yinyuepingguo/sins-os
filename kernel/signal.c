#include <sins/signal.h>
#include <sins/processor.h>
#include <linkage.h>
#include <sins/kernel.h>
#include <sins/sched.h>
#include <sins/syscalls.h>

static void def_sig_handler(unsigned long signal)
{
#ifdef DEBUG
	printk("received signal[%d].", signal);
#endif
	switch (signal) {
	case SIGCHLD:
		/* ignore it */
		break;
	default:
		sys_exit(-1);
		break;
	}
}
static void bad_signal()
{
	printk("bad signal handle.exit!");
	sys_exit(-1);
}

asmlinkage unsigned long sys_sigreturn()
{
	struct pt_regs *regs = 
		(struct pt_regs *)((unsigned long)current - 
			sizeof(struct pt_regs)+PAGE_SIZE);
	unsigned long flags;
	unsigned long prev_ret;
	
	if (current->sig_restorer == SIG_RESTORER_NULL) {
		printk("call sigreturn[there is no signal].");
		sys_exit(-1);
	}
	
	prev_ret = arch_sig_restore(regs);
	if (FAILED(arch_check_user_regs(regs))) {
		printk("sigreturn[invalid regs]");
		sys_exit(-1);
	}

	irq_save(flags);
	current->sig_restorer = SIG_RESTORER_NULL;
	irq_restore(flags);
	return prev_ret;
}

asmlinkage result_t sys_sigaction(unsigned long signal, sig_handler_t s_handler)
{
	if (signal >= NR_SIGNALS)
		return -ERROR;	
	
	if ((unsigned long)s_handler >= PAGE_OFFSET)
		return -ERROR;

	/* if signal is unmaskable. we can't change its behavior */
	if ((BIT(signal) & SIG_UNMASK) != 0)
		return -ERROR;

	current->sig_fn[signal] = s_handler;
	return SUCCESS;
}

asmlinkage result_t sys_sigmask(unsigned long signal, int mask)
{
	if (signal >= NR_SIGNALS)
		return -ERROR;

	/* user are not allowed change unmaskable signal's mask */
	if ((BIT(signal) & SIG_UNMASK) != 0)
		return -ERROR;

	if (mask)
		set_bit(current->sig_mask, signal);
	else
		clear_bit(current->sig_mask, signal);
	return SUCCESS;
}

asmlinkage result_t sys_kill(unsigned long pid, unsigned long signal)
{
	struct task_struct *task;
	unsigned long flags;

	task = get_task_by_pid(pid);	

	if (task && !is_kthread(task)) {
		if (signal >= NR_SIGNALS)
			return -ERROR;
		irq_save(flags);
		set_bit(task->signal, signal);
		irq_restore(flags);
		return SUCCESS;
	}
	return -ERROR;
}

asmlinkage void do_signal(struct pt_regs *regs)
{
	struct task_struct *curr = current;
	unsigned long i;
	unsigned long flags;
	sig_handler_t s_handler;

	/* no signal received, we want the common case go fast. */
	if (likely(curr->signal == 0))
		return;

	/* is necessary.kernel mode program is not allowed to use signal */
	if (from_kernel(regs))
		return;

	/* a kernel thread should never go here */
	BUG_ON(is_kthread(curr));

	/* must wait now sig_handler exec success */
	if (curr->sig_restorer != SIG_RESTORER_NULL)
		return ;

	irq_save(flags);
	for (i = 0; i != NR_SIGNALS; ++i) {
		if (test_bit(curr->signal, i)) {
			break;
		}
	}
	BUG_ON(i >= NR_SIGNALS);

	/* test if we mask this maskable signal */
	if ((curr->sig_mask && BIT(i) && ~SIG_UNMASK) != 0) {
		/* ignore this signal */
		goto finish;
	}
	s_handler = curr->sig_fn[i];
	if (s_handler == NULL) {
		def_sig_handler(i);
		goto finish;
	}

	if (check_user_regs(regs) >= 0) {
		curr->sig_restorer = setup_sig_handler(regs, s_handler);
		if (curr->sig_restorer == SIG_RESTORER_NULL) {
			bad_signal();
			goto finish;
		}
	} else {
		bad_signal();
	}

finish:
	clear_bit(curr->signal, i);
	irq_restore(flags);
}


