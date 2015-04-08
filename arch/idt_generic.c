#include <sins/kernel.h>
#include <sins/irq.h>
#include <sins/processor.h>
#include <sins/syscalls.h>

static void print_eflags(unsigned long eflags)
{
	printk((eflags & X86_EFLAGS_ID)? "ID ": "id ");
	printk((eflags & X86_EFLAGS_VIP)? "VIP ": "vip ");
	printk((eflags & X86_EFLAGS_VIF)? "VIF ": "vif ");
	printk((eflags & X86_EFLAGS_AC)? "AC ": "ac ");
	printk((eflags & X86_EFLAGS_VM)? "VM ": "vm ");
	printk((eflags & X86_EFLAGS_RF)? "RF ": "rf ");
	printk((eflags & X86_EFLAGS_NT)? "NT ": "nt ");
	printk("IOPL=%d ", (eflags & X86_EFLAGS_IOPL)>>13);
	printk((eflags & X86_EFLAGS_OF)? "OF ": "of ");
	printk((eflags & X86_EFLAGS_DF)? "DF ": "df ");
	printk((eflags & X86_EFLAGS_IF)? "IF ": "if ");
	printk((eflags & X86_EFLAGS_TF)? "TF ": "tf ");
	printk((eflags & X86_EFLAGS_SF)? "SF ": "sf ");
	printk((eflags & X86_EFLAGS_ZF)? "ZF ": "zf ");
	printk((eflags & X86_EFLAGS_AF)? "AF ": "af ");
	printk((eflags & X86_EFLAGS_PF)? "PF ": "pf ");
	printk((eflags & X86_EFLAGS_CF)? "CF": "cf");
	printk("\n");
}

void die(const char *str, struct pt_regs *regs, unsigned long error_code)
{
	unsigned long flags;
	unsigned long i;

	/* print some state for debug */
	irq_save(flags);
	printk("\t\t\tkernel RESUME from panic\n");
	for (i = 0; i != 80; ++i)
		printk("%c", '-');
	printk("eax:%p \tebx:%p \tecx:%p \tedx: %p\n",
		regs->eax, regs->ebx, regs->ecx, regs->edx);
	printk("edi:%p \tesi:%p \tebp:%p \n",
		regs->edi, regs->esi, regs->ebp);
	printk(" ds:%p \t es:%p \t fs:%p \t gs:%p\t\n",
		regs->ds, regs->es, regs->fs, regs->gs);
	if (regs->cs & 0x3) {
		printk(" cs:%p \teip:%p \t ss:%p \tesp:%p\n",
			regs->cs, regs->eip, regs->ss, regs->esp);
	} else {
		printk(" cs:%p \teip:%p \t ss:undefined \tesp:undefined\n",
			regs->cs, regs->eip);
	}
	printk("eflags:%p      ", regs->eflags);
	print_eflags(regs->eflags);
	for (i = 0; i != 80; ++i)
		printk("%c", '-');
	printk("panic reason: %s, error code: %p. exit!\n", str, error_code);
	/* never restore irq */
	irq_restore(flags);
	sys_exit(-1);
}

void do_divide_error(struct pt_regs *regs, unsigned long error_code)
{
	die("divide error", regs, error_code);
}

void do_double_fault(struct pt_regs *regs, unsigned long error_code)
{
	die("double fault", regs, error_code);
}

void do_int3(struct pt_regs *regs, unsigned long error_code)
{
	die("int3", regs, error_code);
}

void do_nmi(struct pt_regs *regs, unsigned long error_code)
{
	die("nmi", regs, error_code);
}

void do_debug(struct pt_regs *regs, unsigned long error_code)
{
	die("debug", regs, error_code);
}

void do_overflow(struct pt_regs *regs, unsigned long error_code)
{
	die("overflow", regs, error_code);
}

void do_bounds(struct pt_regs *regs, unsigned long error_code)
{
	die("bounds", regs, error_code);
}

void do_invalid_op(struct pt_regs *regs, unsigned long error_code)
{
	die("invalid op", regs, error_code);
}

void do_device_not_available(struct pt_regs *regs, unsigned long error_code)
{
	die("device not available", regs, error_code);
}

void do_coprocessor_segment_overrun(struct pt_regs *regs, unsigned long error_code)
{
	die("coprocessor segment overrun", regs, error_code);
}

void do_invalid_tss(struct pt_regs *regs, unsigned long error_code)
{
	die("invalid tss", regs, error_code);
}

void do_general_protection(struct pt_regs *regs, unsigned long error_code)
{
	die("general protection", regs, error_code);
}

void do_reserved(struct pt_regs *regs, unsigned long error_code)
{
	printk("reserved", regs, error_code);
}

void do_coprocessor_error(struct pt_regs *regs, unsigned long error_code)
{
	die("coprocessor error", regs, error_code);
}

void do_stack_segment(struct pt_regs *regs, unsigned long error_code)
{
	die("stack segment", regs, error_code);
}

void do_page_fault(struct pt_regs *regs, unsigned long error_code)
{
	die("page fault", regs, error_code);
}

void do_segment_not_present(struct pt_regs *regs, unsigned long error_code)
{
	die("segment not present", regs, error_code);
}

