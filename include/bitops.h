#ifndef _BITOPS_H
#define _BITOPS_H

#include <asm/bitops.h>

#define BIT(nr)		(1UL << (nr))
#define BIT_ULL(nr)	(1ULL << (nr))

#define set_bit(op, bit) ((op) |= BIT(bit))
#define clear_bit(op, bit) ((op) &= (~BIT(bit)))
#define test_bit(op, bit) ((op) & BIT(bit))

#define ffs(x)	arch_ffs(x)
#define fls(x)	arch_fls(x)
#define ffz(x)	arch_ffz(x)
#define flz(x)	arch_flz(x)

#endif
