#include <user/io.h>
#include <user/syscalls.h>

void debug_trap_fault()
{
	asm volatile("int $0x3");
}

asmlinkage void my_signal_handler()
{
	printf("enter my_signal_handler.");	
}

void debug_signal()
{
	getpid();
	sigaction(SIGUSR1, my_signal_handler);
//	ret = kill(getpid(), SIGUSR1);
}

int main(int argc, const char *argv[])
{
	/* This program is used to debug kernel feature */
//	debug_trap_fault();
	debug_signal();
	while(1);
/*	i = kill(getppid(), SIGKILL);
	printf("kill=%d", i);*/
	return 0;
}
