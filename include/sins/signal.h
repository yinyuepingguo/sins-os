#ifndef _SINS_SIGNAL_H
#define _SINS_SIGNAL_H

#include <bitops.h>
#include <linkage.h>
#include <sins/processor.h>

typedef asmlinkage void (*sig_handler_t)();

#define NR_SIGNALS	5

#define SIGALRM	0
#define SIGUSR1	1
#define SIGUSR2	2
#define SIGKILL	3
#define SIGCHLD	4

#define SIG_UNMASK	BIT(SIGKILL)

#define SIG_RESTORER_NULL	ULONG_MAX

extern asmlinkage void do_signal(struct pt_regs *regs);

#include <asm/signal.h>

#define setup_sig_handler(regs, s_handler)	\
	arch_setup_sig_handler(regs, s_handler)
#define sig_restore(regs)	\
	arch_sig_restore(regs)

#endif
