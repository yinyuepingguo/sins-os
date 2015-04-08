#include <sins/signal.h>
#include <sins/syscalls.h>
#include <sins/error.h>
#include <sins/kernel.h>
#include <types.h>
#include <sins/syscalls.h>
#include <string.h>
#include <sins/mm.h>
#include <sins/sched.h>

unsigned long arch_setup_sig_handler
	(struct pt_regs *regs, sig_handler_t  s_handler)
{
	unsigned long *esp;
	unsigned long tmp;
	unsigned long ret;
	const struct {
		u8 movl;
		u32 val;
		u16 int80;
		u8 pad;
	} __attribute__((packed)) retcode = {
		0xb8,
		__NR_sigreturn,
		0x80cd,
		0
	};

	esp = (unsigned long *)regs->esp;
#ifdef DEBUG
	printk("ori_eax:%p ", regs->eax);
	printk("ori_ebx:%p ", regs->ebx);
	printk("ori_ecx:%p \n", regs->ecx);
	printk("ori_edx:%p ", regs->edx);
	printk("ori_edi:%p \n", regs->edi);
	printk("ori_esi:%p ", regs->esi);
	printk("ori_ebp:%p \n", regs->ebp);	
	printk("ori_eip:%p ", regs->eip);
	printk("ori_esp:%p \n", regs->esp);
#endif
	
	/* 
		we don't restore all registers 
		as cs...gs is same for USER MODE.
		and it's not safe.
	 */
	*(--esp) = regs->eax;
	*(--esp) = regs->ebx;
	*(--esp) = regs->ecx;
	*(--esp) = regs->edx;
	*(--esp) = regs->edi;
	*(--esp) = regs->esi;
	*(--esp) = regs->ebp;
	*(--esp) = regs->eip;	/* origin ds. need to check when restore*/
	*(--esp) = regs->eflags;/* es.need check */
	*(--esp) = regs->esp;	/* fs.need check */
	esp -= 2;
	memcpy(esp, &retcode, 2 * sizeof(unsigned long));
	tmp = (unsigned long)esp;
	*(--esp) = tmp;

	ret = (unsigned long)esp;

	//return address
	regs->eip = (unsigned long)s_handler;
	regs->esp = (unsigned long)esp;

	/* can return SIG_RESTORER_NULL if error happened */	
	return ret;
}

unsigned long arch_sig_restore(struct pt_regs *regs)
{
	unsigned long *esp = (unsigned long *)current->sig_restorer;
	
	esp += 3;
	regs->esp = *esp++;
	regs->eflags = *esp++;
	regs->eip = *esp++;
	regs->ebp = *esp++;
	regs->esi = *esp++;
	regs->edi = *esp++;
	regs->edx = *esp++;
	regs->ecx = *esp++;
	regs->ebx = *esp++;
	regs->eax = *esp++;
#ifdef DEBUG
	printk("eax:%p ", regs->eax);
	printk("ori_ebx:%p ", regs->ebx);
	printk("ori_ecx:%p \n", regs->ecx);
	printk("ori_edx:%p ", regs->edx);
	printk("ori_edi:%p \n", regs->edi);
	printk("ori_esi:%p ", regs->esi);
	printk("ori_ebp:%p \n", regs->ebp);	
	printk("ori_eip:%p ", regs->eip);
	printk("ori_esp:%p \n", regs->esp);
#endif
	return regs->eax;
}

result_t arch_check_user_regs(struct pt_regs *regs)
{
	if (regs->esp > PAGE_OFFSET || regs->eip >= PAGE_OFFSET
		|| (regs->eflags&X86_EFLAGS_IOPL) != 0 
		|| (regs->eflags&X86_EFLAGS_IF) == 0)
		return -ERROR;
	return 0;
}
