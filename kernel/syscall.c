#include <sins/syscalls.h>
#include <sins/kernel.h>

void *syscall_table[NR_SYSTEM_CALLS] = {
	sys_open,
	sys_creat,
	sys_close,
	sys_read,
	sys_write,
	sys_ioctl,
	sys_lseek,
	sys_exec,
	sys_exit,
	sys_wait_pid,
	sys_wait_chld,
	sys_chdir,
	sys_chroot,
	sys_getcwd,
	sys_dup,
	sys_dup2,
	sys_readdir,
	sys_sigreturn,
	sys_sigaction,
	sys_sigmask,
	sys_kill,
	sys_getpid,
	sys_getppid,
	sys_time
};

asmlinkage void bad_syscall(unsigned long nr)
{
	printk("unavialable syscall[%u].", nr);
}
