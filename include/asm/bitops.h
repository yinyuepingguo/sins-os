#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H

static inline unsigned long arch_ffs(unsigned long word)
{
	if (word == 0)
		return 0;
	asm("bsfl %1, %0"
		: "=r" (word)
		: "rm" (word));
	return word + 1;
}

static inline unsigned long arch_fls(unsigned long word)
{
	if (word == 0)
		return 0;
	asm("bsrl %1, %0"
		: "=r" (word)
		: "rm" (word));
	return word + 1;
}

static inline unsigned long arch_ffz(unsigned long word)
{
	if (word == 0)
		return 0;
	asm("bsfl %1, %0"
		: "=r" (word)
		: "r" (~word));
	return word + 1;
}

static inline unsigned long arch_flz(unsigned long word)
{
	if (word == 0)
		return 0;
	asm("bsrl %1, %0"
		: "=r" (word)
		: "rm" (~word));
	return word + 1;
}

#endif
