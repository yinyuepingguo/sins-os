#ifndef _SINS_PROCESSOR_H
#define _SINS_PROCESSOR_H

#include <asm/processor.h>

#define halt()	arch_halt()
#define nop()	arch_nop()

#define NR_EXTERNAL_VECTORS ARCH_NR_EXTERNAL_VECTORS

#define pt_regs arch_pt_regs

#define move_to_user_mode(ip, sp, argc, argv)	\
	arch_move_to_user_mode(ip, sp, argc, argv)

#define from_kernel(regs) arch_from_kernel(regs)

#define check_user_regs(regs) arch_check_user_regs(regs)

#endif
