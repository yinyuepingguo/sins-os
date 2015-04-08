#ifndef _TYPES_H
#define _TYPES_H

#define BITS_PER_BYTE	8
#define BITS_PER_LONG	32

#ifndef __ASSEMBLY__

typedef char		s8;
typedef short		s16;
typedef int		s32;
typedef long long	s64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

typedef unsigned long	size_t;
typedef long		ssize_t;

typedef ssize_t result_t;

typedef unsigned char	byte;
typedef long off_t;
typedef long loff_t;

#endif /* not def __ASSEMBLY__ */

#endif
