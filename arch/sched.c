#include <sins/sched.h>
#include <types.h>
#include <sins/processor.h>
#include <string.h>
#include <sins/kernel.h>

extern asmlinkage void kthread_wrapper();
extern asmlinkage void _kthread_wrapper();

void arch_kthread_context_init(struct task_context *context,
	int (*pfn)(unsigned long data),
		unsigned long data, unsigned long stack)
{
	struct pt_regs *regs;

	regs = (struct pt_regs *)(stack - sizeof(struct pt_regs));

	memset(regs, 0, sizeof(struct pt_regs));
	regs->ecx = (u32)pfn;
	regs->edx = data;
	regs->cs = GDT_KERNEL_CS << 3;
	regs->ds = regs->es = regs->fs = regs->gs = GDT_KERNEL_DS << 3;
	regs->eflags = X86_EFLAGS_IF;
	regs->eip = (u32)_kthread_wrapper;
	context->eip = (u32)kthread_wrapper;
	context->esp = (u32)regs;	
}
