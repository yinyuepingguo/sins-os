#include <sins/syscalls.h>

syscall2(int, open, const char *, path, int, flags);
syscall1(int, creat, const char *, path);
syscall1(int, close, unsigned int, fd);
syscall3(ssize_t, read, unsigned int, fd, byte *, buf, size_t, size);
syscall3(ssize_t, write, unsigned int, fd, const byte *, buf, size_t, size);
syscall3(result_t, ioctl, unsigned int, fd,
	unsigned long, cmd, unsigned long, data);
syscall3(loff_t, lseek, unsigned int, fd, loff_t, offset, int, whence);
syscall3(int, exec, const char *, path,
	unsigned long, argc, const char **, argv);
syscall1(int, exit, result_t, code);
syscall1(result_t, wait_pid, unsigned long, pid);
syscall1(result_t, wait_chld, unsigned long *, pid);
syscall1(result_t, chdir, const char *, path);
syscall1(result_t, chroot, const char *, path);
syscall2(result_t, getcwd, char *, path, unsigned long, len);
syscall1(int, dup, unsigned int, old_fd);
syscall2(int, dup2, unsigned int, old_fd, unsigned int, new_fd);
syscall3(ssize_t, readdir, unsigned int, fd, struct dir *, buf, size_t, size);
syscall0(int, sigreturn);
syscall2(result_t, sigaction, unsigned long, signal, sig_handler_t, s_handler);
syscall2(result_t, sigmask, unsigned long, signal, int, mask);
syscall2(result_t, kill, unsigned long, pid, unsigned long, signal);
syscall0(unsigned long, getpid);
syscall0(unsigned long, getppid);
syscall1(time_t, time, struct tm *, tm);
