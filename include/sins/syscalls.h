#ifndef _SINS_SYSCALLS_H
#define _SINS_SYSCALLS_H

#ifndef __ASSEMBLY__

#include <linkage.h>
#include <sins/kernel.h>
#include <sins/signal.h>
#include <sins/fs.h>
#include <sins/time.h>

extern asmlinkage int sys_open(const char *__user filename, int flags);
extern asmlinkage int sys_creat(const char *__user pathname);
extern asmlinkage int sys_close(unsigned int fd);
extern asmlinkage ssize_t sys_read(unsigned int, byte __user *, size_t);
extern asmlinkage ssize_t sys_write(unsigned int, const byte __user *, size_t);
extern asmlinkage result_t sys_ioctl(unsigned int,
	unsigned long, unsigned long );
extern asmlinkage loff_t sys_lseek(unsigned int, loff_t, int);

extern asmlinkage int sys_exec(const char *__user filename,
	unsigned long argc, const char *__user argv[]);
extern asmlinkage void sys_exit(result_t code);
extern asmlinkage result_t sys_wait_pid(unsigned long pid);
extern asmlinkage result_t sys_wait_chld(unsigned long *pid);

extern asmlinkage result_t sys_chdir(const char *__user filename);
extern asmlinkage result_t sys_chroot(const char *__user filename);
extern asmlinkage result_t sys_getcwd(char *__user path, unsigned long len);

extern asmlinkage int sys_dup(unsigned int old_fd);
extern asmlinkage int sys_dup2(unsigned int old_fd, unsigned int new_fd);

extern asmlinkage ssize_t sys_readdir
	(unsigned int fd, struct dir *__user buf, size_t len);

extern asmlinkage unsigned long sys_sigreturn();
extern asmlinkage result_t sys_sigaction
	(unsigned long signal, sig_handler_t s_handler);
extern asmlinkage result_t sys_sigmask(unsigned long signal, int mask);
extern asmlinkage result_t sys_kill(unsigned long pid, unsigned long signal);

extern asmlinkage unsigned long sys_getpid();
extern asmlinkage unsigned long sys_getppid();

extern asmlinkage time_t sys_time(struct tm *tm);

extern asmlinkage void bad_syscall(unsigned long nr);

#endif /* not def __ASSEMBLY__ */

#define __NR_open	0
#define __NR_creat	1
#define __NR_close	2
#define __NR_read	3
#define __NR_write	4
#define __NR_ioctl	5
#define __NR_lseek	6
#define __NR_exec	7
#define __NR_exit	8
#define __NR_wait_pid	9
#define __NR_wait_chld	10
#define __NR_chdir	11
#define __NR_chroot	12
#define __NR_getcwd	13
#define __NR_dup	14
#define __NR_dup2	15
#define __NR_readdir	16
#define __NR_sigreturn	17
#define __NR_sigaction	18
#define __NR_sigmask	19
#define __NR_kill	20
#define __NR_getpid	21
#define __NR_getppid	22
#define __NR_time	23

#define NR_SYSTEM_CALLS	24

/* now used ebx,ecx,edx,esi */

#define syscall0(type,name) \
type name(void) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name)); \
return __res; \
}

#define syscall1(type,name,atype,a) \
type name(atype a) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a)); \
return __res; \
}

#define syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a),"c" (b)); \
return __res; \
}

#define syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a),"c" (b),"d" (c)); \
return __res;\
}

#endif
