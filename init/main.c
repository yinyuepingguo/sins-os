#include <linkage.h>
#include <sins/irq.h>
#include <sins/boot_params.h>
#include <sins/init.h>
#include <sins/console.h>
#include <sins/mm.h>
#include <sins/timer.h>
#include <sins/sched.h>
#include <sins/processor.h>
#include <user/syscalls.h>
#include <sins/kernel.h>
#include <user/io.h>
#include <sins/time.h>

static int init(unsigned long data)
{
	result_t ret;
	int tty_fd;
	unsigned long chld_pid;

	do_device_initcalls();

	/* open STDIN */
	tty_fd = open("/dev/tty", 0);
	if (FAILED(tty_fd)) {
		BUG("/dev/tty could not be opened.");
	}
	dup(tty_fd); /* STDOUT */
	dup(tty_fd); /* STDERR */

	ret = exec("/bin/sh", 0, NULL);

	close(STDIN);
	close(STDOUT);
	close(STDERR);

	if (FAILED(ret)) {
		panic("exec returns %d.\n"
			"Please add 'sh' program to disk.\n", ret);
	}

	while(1) {
		/* could do some system management task */
		ret = wait_chld(&chld_pid);
#ifndef DEBUG
		printk("INIT:child[%d] returns %d.", chld_pid, ret);
#endif
		halt();	
	}
	return SUCCESS;
}

asmlinkage void start_kernel(unsigned long magic, unsigned long addr)
{
	irq_disable();
	do_pure_initcalls();
	/* we want initilize time early. */
	time_init();
	parse_boot_params(magic, addr);
	timer_init();
	do_arch_initcalls();
	do_archdone_initcalls();
	mm_init();
	sched_init();
	do_subsys_initcalls();
	do_fs_initcalls();
	irq_enable();
	kernel_thread("init", init, 0);

	while(1) {
		/* could do some system management task */
		halt();	
	}
}
