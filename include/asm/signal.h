#ifndef _ASM_SIGNAL_H
#define _ASM_SIGNAL_H

#include <sins/processor.h>

extern unsigned long arch_setup_sig_handler
	(struct pt_regs *regs, sig_handler_t s_handler);
extern unsigned long arch_sig_restore(struct pt_regs *regs);

#endif
