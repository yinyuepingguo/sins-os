#ifndef _SINS_INIT_H
#define _SINS_INIT_H

#include <sins/kernel.h>

typedef struct {
	char *name;
	result_t (*handler)(void);
} initcall_t;

#define __define_initcall(level,fn,id) \
	static initcall_t __initcall_##fn##id __used \
	__attribute__((__section__(".initcall" level ".init")))	= {#fn, fn}

#define pure_initcall(fn)	__define_initcall("0", fn, 0)
#define arch_initcall(fn)	__define_initcall("1", fn, 1)
#define archdone_initcall(fn)	__define_initcall("2", fn, 2)
#define subsys_initcall(fn)	__define_initcall("3", fn, 3)
#define fs_initcall(fn)		__define_initcall("4", fn, 4)
#define device_initcall(fn)	__define_initcall("5", fn, 5)

extern initcall_t __initcall[], __initcall_end[];
extern initcall_t __initcall_pure[], __initcall_pure_end[];
extern initcall_t __initcall_arch[], __initcall_arch_end[];
extern initcall_t __initcall_archdone[], __initcall_archdone_end[];
extern initcall_t __initcall_subsys[], __initcall_subsys_end[];
extern initcall_t __initcall_fs[], __initcall_fs_end[];
extern initcall_t __initcall_device[], __initcall_device_end[];

extern void do_initcalls(initcall_t *begin, initcall_t *end); 

#define do_pure_initcalls()	\
	do_initcalls(__initcall_pure, __initcall_pure_end)
#define do_arch_initcalls()	\
	do_initcalls(__initcall_arch, __initcall_arch_end)
#define do_archdone_initcalls()	\
	do_initcalls(__initcall_archdone, __initcall_archdone_end)
#define do_subsys_initcalls()	\
	do_initcalls(__initcall_subsys, __initcall_subsys_end)
#define do_fs_initcalls()	\
	do_initcalls(__initcall_fs, __initcall_fs_end)
#define do_device_initcalls()	\
	do_initcalls(__initcall_device, __initcall_device_end)

#endif
