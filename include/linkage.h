#ifndef _LINKAGE_H
#define _LINKAGE_H

#ifndef __ASSEMBLY__

#define asmlinkage __attribute__((regparm(0)))
#define fastcall __attribute__((regparm(3)))

#endif /* not def __ASSEMBLY__ */

#ifdef __ASSEMBLY__

#define ENTRY(name)	\
	.globl name;	\
	.align 4;	\
	name:
#define END(name)
#define ENDPROC(name)

#endif /* __ASSEMBLY__ */

#endif
